#include "StateMachine.h"
#include "Util/stdafx.h"

StateMachine::StateMachine() : Threaded("StateMachine", 4 * 1024) {}

void StateMachine::loop() {
	if (currentState == nullptr) {
		return;
	}

	currentState->loop();
}

void StateMachine::begin() {
	start();
}

StateMachine::~StateMachine(){
	if(currentState == nullptr) return;

	stop(0);
	currentState->unblock();
	while(running()){
		delayMillis(1);
	}
	delete currentState;
	currentState = nullptr;
}