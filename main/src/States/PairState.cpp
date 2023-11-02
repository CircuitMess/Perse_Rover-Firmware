#include "PairState.h"
#include "Util/Services.h"
#include "DriveState.h"
#include "Services/LEDService.h"
#include "Devices/Input.h"

PairState::PairState() : State(), queue(10) {
	Events::listen(Facility::Input, &queue);
	Events::listen(Facility::Pair, &queue);

	if (LEDService* led = (LEDService*)Services.get(Service::LED)) {
		led->on(LED::StatusYellow);
	}
}

PairState::~PairState() {
	if (LEDService* led = (LEDService*)Services.get(Service::LED)) {
		led->off(LED::StatusYellow);
	}

	Events::unlisten(&queue);
}

void PairState::loop() {
	LEDService* led = (LEDService*)Services.get(Service::LED);
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

			led->blink(LED::StatusYellow, 0);
			led->off(LED::StatusRed);
		}
		else {
			led->on(LED::StatusYellow);
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