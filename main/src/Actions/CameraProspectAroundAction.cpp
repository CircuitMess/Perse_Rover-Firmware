#include "CameraProspectAroundAction.h"
#include "Util/stdafx.h"
#include "Util/Services.h"
#include "Devices/CameraController.h"

CameraProspectAroundAction::CameraProspectAroundAction() : startTime(millis()){
	controller = (CameraController*) Services.get(Service::CameraController);

	if(controller == nullptr){
		return;
	}

	controller->setControl(DeviceControlType::Local);
}

CameraProspectAroundAction::~CameraProspectAroundAction(){
	if(controller == nullptr){
		return;
	}

	controller->setControl(DeviceControlType::Remote);
}

void CameraProspectAroundAction::loop(){
	PlayAudioAction::loop();

	if(controller == nullptr){
		markForDestroy();
		return;
	}

	if (readyToTransition()){
		markForDestroy();
		return;
	}

	CameraState state{ .Rotation = 0 };

	const uint64_t deltaTime = millis() - startTime;
	const uint8_t section = deltaTime / RotateDelay;

	if(section == 1){
		state.Rotation = 100;
	}
	else if (section > 1){
		state.Rotation = 50;
	}

	controller->setLocally(state);
}

bool CameraProspectAroundAction::readyToTransition() const{
	return millis() - startTime >= 3 * RotateDelay;
}
