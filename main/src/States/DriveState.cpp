#include "DriveState.h"
#include "Pins.hpp"
#include "Services/TCPServer.h"
#include "PairState.h"
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

DriveState::DriveState() : State(), queue(10), activeAction(nullptr){
	Events::listen(Facility::TCP, &queue);
	Events::listen(Facility::Feed, &queue);
	Events::listen(Facility::Comm, &queue);

	if (LEDService* led = (LEDService*)Services.get(Service::LED)) {
		led->on(LED::StatusGreen);
	}
}

DriveState::~DriveState() {
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
				}
			}
		}else if(event.facility == Facility::Feed){
			if(auto* feedEvent = (Feed::Event*) event.data){
				if(feedEvent->type == Feed::EventType::MarkerScanned){
					if(actionMappings.contains(feedEvent->markerAction) && actionMappings.at(feedEvent->markerAction) != nullptr){
						if(activeAction == nullptr || activeAction->readyToTransition()){
							activeAction = actionMappings.at(feedEvent->markerAction)();
						}
					}
				}
			}
		}else if(event.facility == Facility::Comm){
			if(const Comm::Event* commEvent = (Comm::Event*)event.data){
				if(commEvent->type == CommType::Emergency && commEvent->emergency){
					activeAction = std::make_unique<PanicAction>();
				}else if(commEvent->type == CommType::Audio){
					if(Audio* audio = (Audio*) Services.get(Service::Audio)){
						audio->setEnabled(commEvent->audio);
						if(commEvent->audio){
							audio->play("/spiffs/audioOn.wav"); //TODO - play beep sound or something
						}
					}
				}
			}
		}

		free(event.data);
	}

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