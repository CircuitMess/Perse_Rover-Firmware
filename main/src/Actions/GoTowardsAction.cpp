#include "GoTowardsAction.h"
#include "Devices/MotorDriveController.h"
#include "Util/Services.h"

GoTowardsAction::GoTowardsAction(){
	controller = (MotorDriveController*)Services.get(Service::MotorDriveController);

	if (controller == nullptr){
		return;
	}

	controller->setControl(DeviceControlType::Local);
}

GoTowardsAction::~GoTowardsAction(){
	if (controller == nullptr){
		return;
	}

	controller->setLocally({});
	controller->setControl(DeviceControlType::Remote);
}

void GoTowardsAction::loop(){
	PlayAudioAction::loop();

	if (controller == nullptr){
		markForDestroy();
		return;
	}

	const MotorDriveState motorDriveState{.DriveDirection = {0, 1.0f}};
	controller->setLocally(motorDriveState);
}
