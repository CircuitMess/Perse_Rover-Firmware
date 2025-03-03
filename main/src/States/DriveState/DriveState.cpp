#include "DriveState.h"
#include "Pins.hpp"
#include "Services/TCPServer.h"
#include "Util/Services.h"
#include "Actions/Action.h"
#include "Actions/GoTowardsAction.h"
#include "Actions/Rotate180Action.h"
#include "Actions/AlertAction.h"
#include "Actions/CallSampleLander.h"
#include "Actions/CameraProspectAroundAction.h"
#include "Actions/HeadlightsSwitchAction.h"
#include "Actions/LifeDetectedAction.h"
#include "Actions/RadioToIngenuityAction.h"
#include "Actions/RendezvousPoint.h"
#include "Actions/TurnLeftGoAheadAction.h"
#include "Actions/TurnRightGoAheadAction.h"
#include "Actions/TakeSoilSampleAction.h"
#include "Services/Feed.h"
#include "Services/LEDService.h"
#include "Services/Comm.h"
#include "Actions/PanicAction.h"
#include "Services/Audio.h"
#include "States/PairState.h"
#include "Devices/Battery.h"
#include "Devices/Input.h"
#include "Util/stdafx.h"
#include "Settings.h"

const std::map<MarkerAction, std::function<std::unique_ptr<Action>(void)>> DriveState::actionMappings = {
		{ MarkerAction::None,                 []() -> std::unique_ptr<Action>{ return std::make_unique<Action>(); }},
		{ MarkerAction::TurnRightGoAhead,     []() -> std::unique_ptr<Action>{ return std::make_unique<TurnRightGoAheadAction>(); }},
		{ MarkerAction::RadioToIngenuity,     []() -> std::unique_ptr<Action>{ return std::make_unique<RadioToIngenuityAction>(); }},
		{ MarkerAction::TakeSoilSample,       []() -> std::unique_ptr<Action>{ return std::make_unique<TakeSoilSampleAction>(); }},
		{ MarkerAction::GoTowards,            []() -> std::unique_ptr<Action>{ return std::make_unique<GoTowardsAction>(); }},
		{ MarkerAction::Rotate180,            []() -> std::unique_ptr<Action>{ return std::make_unique<Rotate180Action>(); }},
		{ MarkerAction::CameraProspectAround, []() -> std::unique_ptr<Action>{ return std::make_unique<CameraProspectAroundAction>(); }},
		{ MarkerAction::HeadlightsSwitch,     []() -> std::unique_ptr<Action>{ return std::make_unique<HeadlightsSwitchAction>(); }},
		{ MarkerAction::LifeDetected,         []() -> std::unique_ptr<Action>{ return std::make_unique<LifeDetectedAction>(); }},
		{ MarkerAction::RendezvousPoint,      []() -> std::unique_ptr<Action>{ return std::make_unique<RendezvousPointAction>(); }},
		{ MarkerAction::Alert,                []() -> std::unique_ptr<Action>{ return std::make_unique<AlertAction>(); }},
		{ MarkerAction::CallSampleLander,     []() -> std::unique_ptr<Action>{ return std::make_unique<CallSampleLanderAction>(); }},
		{ MarkerAction::TurnLeftGoAhead,      []() -> std::unique_ptr<Action>{ return std::make_unique<TurnLeftGoAheadAction>(); }}
};

const std::unordered_set<CommType> DriveState::IdleResetComms = {
		CommType::DriveDir,
		CommType::Headlights,
		CommType::ArmPosition,
		CommType::ArmPinch,
		CommType::CameraRotation,
		CommType::ScanMarkers,
		CommType::Emergency,
		CommType::Audio,
		CommType::ModulePlug
};

DriveState::DriveState() : State(), queue(10), activeAction(nullptr), audio(*(Audio*) Services.get(Service::Audio)){
	if(const TCPServer* tcp = (TCPServer*) Services.get(Service::TCP)){
		if(!tcp->isConnected()){
			if(StateMachine* parentStateMachine = (StateMachine*) Services.get(Service::StateMachine)){
				parentStateMachine->transition<PairState>();
				return;
			}
		}
	}

	Events::listen(Facility::TCP, &queue);
	Events::listen(Facility::Feed, &queue);
	Events::listen(Facility::Comm, &queue);
	Events::listen(Facility::Input, &queue);

	if (LEDService* led = (LEDService*)Services.get(Service::LED)) {
		led->breathe(LED::StatusGreen);
		led->breathe(LED::Rear);
	}

	lastSetMillis = millis();
	if(Settings* settings = (Settings*)Services.get(Service::Settings)){
		camFlip = settings->get().cameraHorizontalFlip;
	}
}

DriveState::~DriveState() {
	activeAction.reset();

	if (LEDService* led = (LEDService*)Services.get(Service::LED)) {
		led->off(LED::StatusGreen);
		led->on(LED::StatusRed);
	}

	Events::unlisten(&queue);
}

void DriveState::loop(){
	bool shouldTransition = false;

	Event event = {};
	if(queue.get(event, 1)){
		if(event.facility == Facility::TCP){
			if(auto* tcpEvent = (TCPServer::Event*) event.data){
				if(tcpEvent->status == TCPServer::Event::Status::Disconnected){
					shouldTransition = true;
					audio.play("/spiffs/General/SignalLost.aac", true);
				}
			}
		}else if(event.facility == Facility::Feed){
			if(auto* feedEvent = (Feed::Event*) event.data){
				if(feedEvent->type == Feed::EventType::MarkerScanned){
					if(actionMappings.contains(feedEvent->markerAction) && actionMappings.at(feedEvent->markerAction) != nullptr){
						if(activeAction == nullptr || activeAction->readyToTransition()){
							activeAction = actionMappings.at(feedEvent->markerAction)();
							randSoundPlayer.resetTimer();
						}
					}
				}
			}
		}else if(event.facility == Facility::Comm){
			if(const Comm::Event* commEvent = (Comm::Event*) event.data){
				if(IdleResetComms.contains(commEvent->type)){
					randSoundPlayer.resetTimer();
				}

				if(commEvent->type == CommType::Emergency && commEvent->emergency){
					activeAction = std::make_unique<PanicAction>();
				}else if(commEvent->type == CommType::Audio){
					audio.setEnabled(commEvent->audio);
					if(commEvent->audio){
						audio.play("/spiffs/Beep3.aac", true);
					}
				}else if(commEvent->type == CommType::ControllerBatteryCritical){
					if(commEvent->controllerBatteryCritical && audio.getCurrentPlayingFile() != "/spiffs/General/BattEmptyCtrl.aac"){
						audio.play("/spiffs/General/BattEmptyCtrl.aac", true);
					}
				}else if(commEvent->type == CommType::ConnectionStrength){
					if(commEvent->connectionStrength == ConnectionStrength::VeryLow && audio.getCurrentPlayingFile() != "/spiffs/General/SignalWeak.aac"){
						audio.play("/spiffs/General/SignalWeak.aac", true);
					}
				}else if(commEvent->type == CommType::ArmControl){
					if(commEvent->armEnabled && audio.getCurrentPlayingFile() != "/spiffs/Systems/ArmOn.aac"){
						audio.play("/spiffs/Systems/ArmOn.aac");
					}else if(!commEvent->armEnabled && audio.getCurrentPlayingFile() != "/spiffs/Systems/ArmOff.aac"){
						audio.play("/spiffs/Systems/ArmOff.aac");
					}
				}
			}
		}else if (event.facility == Facility::Input) {
			const Input::Data* data = (Input::Data*)event.data;
			if(data != nullptr && data->action == Input::Data::Press && millis() - lastSetMillis >= CamFlipPause){
				camFlip = !camFlip;
				lastSetMillis = millis();

				if(audio.getCurrentPlayingFile() != "/spiffs/General/CamFlip.aac"){
					audio.play("/spiffs/General/CamFlip.aac");
				}

				if(auto feed = (Feed*)Services.get(Service::Feed)){
					feed->flipCam(camFlip);
				}

				if(Settings* settings = (Settings*)Services.get(Service::Settings)){
					auto setts = settings->get();
					setts.cameraHorizontalFlip = camFlip;
					settings->set(setts);
					settings->store();
				}
			}
		}

		free(event.data);
	}

	randSoundPlayer.loop();

	if(activeAction != nullptr){
		if(activeAction->isMarkedForDestroy()){
			activeAction.reset();
		}else{
			activeAction->loop();
		}
	}

	if(shouldTransition){
		if(auto parentStateMachine = (StateMachine*) Services.get(Service::StateMachine)){
			parentStateMachine->transition<PairState>();
			return;
		}
	}
}