#include "DriveState.h"
#include "Pins.hpp"
#include "Services/TCPServer.h"
#include "PairState.h"
#include "Util/Services.h"
#include "Services/LED.h"
#include "Actions/Action.h"
#include "Actions/ForwardAction.h"
#include "Actions/Rotate180Action.h"
#include "Services/Feed.h"

const std::map<MarkerAction, std::function<std::unique_ptr<Action>(void)>> DriveState::actionMappings = {
		{ MarkerAction::None, []() -> std::unique_ptr<Action>{ return std::make_unique<Action>(); }},
		{ MarkerAction::Forward, []() -> std::unique_ptr<Action>{ return std::make_unique<ForwardAction>(); }},
		{ MarkerAction::Rotate180, []() -> std::unique_ptr<Action>{ return std::make_unique<Rotate180Action>(); }},
};

DriveState::DriveState() : State(), queue(10){
	Events::listen(Facility::TCP, &queue);
	Events::listen(Facility::Feed, &queue);

	if(auto led = (LED*) Services.get(Service::LED)){
		led->on(EXP_GOOD_TO_GO_LED);
	}
}

DriveState::~DriveState(){
	if(auto led = (LED*) Services.get(Service::LED)){
		led->off(EXP_GOOD_TO_GO_LED);
		led->on(EXP_ERROR_LED);
	}

	Events::unlisten(&queue);
}

void DriveState::loop(){
	Event event = {};
	if(queue.get(event, 1)){
		bool shouldTransition = false;

		if(event.facility == Facility::TCP){
			if(auto* tcpEvent = (TCPServer::Event*) event.data){
				if(tcpEvent->status == TCPServer::Event::Status::Disconnected){
					shouldTransition = true;
				}
			}
		}
		else if (event.facility == Facility::Feed){
			if (auto* feedEvent = (Feed::Event*) event.data){
				if (feedEvent->type == Feed::EventType::MarkerScanned) {
					if (actionMappings.contains(feedEvent->markerAction) && actionMappings.at(feedEvent->markerAction)) {
						activeAction = actionMappings.at(feedEvent->markerAction)();
					}
				}
			}
		}

		free(event.data);

		if(shouldTransition){
			if(auto parentStateMachine = (StateMachine*) Services.get(Service::StateMachine)){
				parentStateMachine->transition<PairState>();
			}
		}
	}

	if(activeAction != nullptr){
		activeAction->loop();
	}
}