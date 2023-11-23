#include "TakeSoilSampleAction.h"
#include "Util/Services.h"
#include "Util/stdafx.h"
#include "Devices/ArmController.h"

TakeSoilSampleAction::TakeSoilSampleAction() : startTime(millis()){
	controller = (ArmController*)Services.get(Service::ArmController);

	if (controller == nullptr) {
		return;
	}

	controller->setControl(DeviceControlType::Local);
}

TakeSoilSampleAction::~TakeSoilSampleAction(){
	if (controller == nullptr){
		return;
	}

	controller->setControl(DeviceControlType::Remote);
}

void TakeSoilSampleAction::loop(){
	if (controller == nullptr){
		markForDestroy();
		return;
	}

	const uint64_t deltaTime = millis() - startTime;

	ArmState state;

	if (deltaTime <= ArmMoveDuration / 2){
		state.Position = StartingArmPosition;
		state.Pinch = StartingArmPinch;
	}
	else if (deltaTime <= ArmMoveDuration){
		state.Position = TargetArmPosition;
	}
	else if (deltaTime - ArmMoveDuration <= PinchDuration){
		state.Pinch = TargetArmPinch;
	}
	else {
		markForDestroy();
		return;
	}

	controller->setLocally(state);
}

bool TakeSoilSampleAction::readyToTransition() const{
	return millis() - startTime >= ArmMoveDuration + PinchDuration;
}
