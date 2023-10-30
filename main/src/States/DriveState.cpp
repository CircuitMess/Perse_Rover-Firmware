#include "DriveState.h"
#include "Pins.hpp"
#include "Services/TCPServer.h"
#include "PairState.h"
#include "Util/Services.h"
#include "Services/LED.h"

DriveState::DriveState() : State(), queue(10) {
	Events::listen(Facility::TCP, &queue);

	if (auto led = (LED*)Services.get(Service::LED)) {
		led->on(EXP_GOOD_TO_GO_LED);
	}
}

DriveState::~DriveState() {
	if (auto led = (LED*)Services.get(Service::LED)) {
		led->off(EXP_GOOD_TO_GO_LED);
		led->on(EXP_ERROR_LED);
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