#include "PairState.h"
#include "Util/Services.h"
#include "DriveState.h"
#include "Pins.hpp"
#include "Services/LED.h"
#include "Devices/Input.h"

PairState::PairState() : State(), queue(10) {
	Events::listen(Facility::Input, &queue);
	Events::listen(Facility::Pair);

	if (auto led = (LED*)Services.get(Service::LED)) {
		led->on(EXP_STANDBY_LED);
	}
}

PairState::~PairState() {
	if (auto led = (LED*)Services.get(Service::LED)) {
		led->off(EXP_STANDBY_LED);
	}

	Events::unlisten(&queue);
}

void PairState::loop() {
	auto led = (LED*)Services.get(Service::LED);
	if (led == nullptr) {
		return;
	}

	Event event{};
	if (!queue.get(event, portMAX_DELAY)) {

	}

	auto* data = (Input::Data*)event.data;
	if (data == nullptr || data->action == Input::Data::Release) {
		led->on(EXP_STANDBY_LED);
		free(event.data);
		pairService.reset(nullptr);
		return;
	}

	led->blinkCont(EXP_STANDBY_LED);
	led->off(EXP_ERROR_LED);
	free(event.data);

	if (pairService == nullptr) {
		pairService = std::make_unique<PairService>();
	}

	if (pairService->getState() == PairService::State::Success) {
		auto* stateMachine = (StateMachine*)Services.get(Service::StateMachine);
		if (stateMachine == nullptr) {
			return;
		}

		stateMachine->transition<DriveState>();
	}
}