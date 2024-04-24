#include "TurnLeftGoAheadAction.h"
#include "Util/stdafx.h"
#include "Devices/MotorDriveController.h"
#include "Util/Services.h"
#include "Devices/Battery.h"

TurnLeftGoAheadAction::TurnLeftGoAheadAction() : startTime(millis()){
	controller = (MotorDriveController*) Services.get(Service::MotorDriveController);

	if(controller == nullptr){
		return;
	}

	controller->setControl(DeviceControlType::Local);
}

TurnLeftGoAheadAction::~TurnLeftGoAheadAction(){
	if(controller == nullptr){
		return;
	}

	controller->setControl(DeviceControlType::Remote);
}

void TurnLeftGoAheadAction::loop(){
	PlayAudioAction::loop();

	if(controller == nullptr){
		markForDestroy();
		return;
	}

	const uint64_t deltaTime = millis() - startTime;
	MotorDriveState state{ .DriveDirection = { .dir = 0, .speed = 1.0f }};

	Battery* battery = (Battery*)Services.get(Service::Battery);
	if (battery == nullptr){
		markForDestroy();
		return;
	}

	const uint64_t turnDuration = TurnDurationAtFull + (1.0f - std::clamp(battery->getPerc(), (uint8_t)0, (uint8_t)100) / 100.0f) * (TurnDurationAtEmpty - TurnDurationAtFull);

	if(deltaTime < turnDuration){
		state.DriveDirection.dir = 6;
	}else if(deltaTime - turnDuration >= ForwardDuration){
		state.DriveDirection.speed = 0.0f;
		markForDestroy();
	}

	controller->setLocally(state);
}

bool TurnLeftGoAheadAction::readyToTransition() const{
	Battery* battery = (Battery*)Services.get(Service::Battery);
	if (battery == nullptr){
		return true;
	}

	const uint64_t turnDuration = TurnDurationAtFull + (1.0f - std::clamp(battery->getPerc(), (uint8_t)0, (uint8_t)100) / 100.0f) * (TurnDurationAtEmpty - TurnDurationAtFull);

	return millis() - startTime >= turnDuration + ForwardDuration;
}
