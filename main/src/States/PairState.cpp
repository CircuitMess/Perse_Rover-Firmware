#include "PairState.h"
#include "Util/Services.h"
#include "DriveState.h"
#include "Pins.hpp"
#include "Services/LED.h"
#include "Devices/Input.h"

PairState::PairState() : State(), queue(10) {
	Events::listen(Facility::Input, &queue);
	Events::listen(Facility::Pair, &queue);

	if (auto led = (LED*)Services.get(Service::LED)) {
		led->on(EXP_STANDBY_LED);
	}

	if(auto input = (Input*) Services.get(Service::Input)){
		if(input->getState(Input::Pair)){
			startPair();
		}
	}
}

PairState::~PairState() {
	if (auto led = (LED*)Services.get(Service::LED)) {
		led->off(EXP_STANDBY_LED);
	}

	Events::unlisten(&queue);
}

void PairState::loop() {
	Event event{};
	if (!queue.get(event, portMAX_DELAY)) {
		return;
	}

	if (event.facility == Facility::Input) {
		const Input::Data* data = (Input::Data*)event.data;
		if(data != nullptr && data->action == Input::Data::Press){
			startPair();
		}else{
			stopPair();
		}
	}
	else if (event.facility == Facility::Pair) {
		const PairService::Event* pairEvent = (PairService::Event*)event.data;
		if (pairEvent != nullptr && pairEvent->success) {
			if (StateMachine* stateMachine = (StateMachine*)Services.get(Service::StateMachine)) {
				stateMachine->transition<DriveState>();
			}
		}
	}

	free(event.data);
}

void PairState::startPair(){
	if(pairService) return;

	pairService = std::make_unique<PairService>();

	if(auto led = (LED*) Services.get(Service::LED)){
		led->off(EXP_ERROR_LED);
		led->blinkCont(EXP_STANDBY_LED);
	}
}

void PairState::stopPair(){
	if(!pairService) return;

	pairService.reset();

	if(auto led = (LED*) Services.get(Service::LED)){
		led->on(EXP_STANDBY_LED);
	}
}
