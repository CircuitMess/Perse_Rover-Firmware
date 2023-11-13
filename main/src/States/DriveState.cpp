#include "DriveState.h"
#include "Services/TCPServer.h"
#include "PairState.h"
#include "Util/Services.h"
#include "Services/LEDService.h"

DriveState::DriveState() : State(), queue(10) {
	Events::listen(Facility::TCP, &queue);

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

void DriveState::loop() {
	Event event = {};
	if (!queue.get(event, portMAX_DELAY))
	{
		return;
	}

	bool shouldTransition = false;

	if (event.facility == Facility::TCP) {
		if (auto* tcpEvent = (TCPServer::Event*)event.data) {
			if (tcpEvent->status == TCPServer::Event::Status::Disconnected) {
				shouldTransition = true;
			}
		}
	}

	free(event.data);

	if (shouldTransition) {
		if (auto parentStateMaching = (StateMachine*)Services.get(Service::StateMachine)) {
			parentStateMaching->transition<PairState>();
		}
	}
}