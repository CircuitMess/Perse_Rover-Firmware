#include "PairState.h"
#include "Util/Services.h"
#include "DriveState.h"
#include "Pins.hpp"
#include "Services/LED.h"
#include "Devices/Input.h"

PairState::PairState() : State(), queue(10) {
	Events::listen(Facility::Input, &queue);

	if (auto led = (LED*)Services.get(Service::LED)) {
		led->on(EXP_STANDBY_LED);
	}
}

PairState::~PairState() {
	if (auto led = (LED*)Services.get(Service::LED)) {
		led->off(EXP_STANDBY_LED);
	}

	delete pairService;
}

void PairState::loop() {
	auto led = (LED*)Services.get(Service::LED);
	if (led == nullptr) {
		return;
	}

	Event event{};
	if (queue.get(event, 10)) {
		auto* data = (Input::Data*)event.data;
		if (data == nullptr || data->action == Input::Data::Release) {
			led->on(EXP_STANDBY_LED);
			free(event.data);
			return;
		}

		led->blinkCont(EXP_STANDBY_LED);
		led->off(EXP_ERROR_LED);
		free(event.data);
	}

	if (pairService == nullptr) {
		pairService = new PairService();
	}

	if (pairService->getState() == PairService::State::Success) {
		auto* stateMachine = (StateMachine*)Services.get(Service::StateMachine);
		if (stateMachine == nullptr) {
			return;
		}

		stateMachine->transition<DriveState>();
	}
}