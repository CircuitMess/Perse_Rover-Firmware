#include "StateMachine.h"

StateMachine::StateMachine() : Threaded("StateMachine", 16 * 1024) {}

void StateMachine::loop() {
	if (currentState == nullptr) {
		return;
	}

	currentState->loop();
}

void StateMachine::begin() {
	start();
}
