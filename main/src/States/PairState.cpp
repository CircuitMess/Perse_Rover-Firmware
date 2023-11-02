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
		led->on(EXP_LED_STATUS_YELLOW);
	}
}

PairState::~PairState() {
	if (auto led = (LED*)Services.get(Service::LED)) {
		led->off(EXP_LED_STATUS_YELLOW);
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
		return;
	}

	if (event.facility == Facility::Input) {
		const Input::Data* data = (Input::Data*)event.data;
		if (data != nullptr && data->action == Input::Data::Press) {
			if (pairService == nullptr) {
				pairService = std::make_unique<PairService>();
			}

			led->blinkCont(EXP_LED_STATUS_YELLOW);
			led->off(EXP_LED_STATUS_RED);
		}
		else {
			led->on(EXP_LED_STATUS_YELLOW);
			pairService.reset(nullptr);
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