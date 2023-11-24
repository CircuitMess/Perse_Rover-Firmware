#include "Rotate180Action.h"
#include "Util/Services.h"
#include "Devices/MotorDriveController.h"
#include "Util/stdafx.h"
#include "Devices/Battery.h"

Rotate180Action::Rotate180Action() : startTime(millis()){
	controller = (MotorDriveController*) Services.get(Service::MotorDriveController);

	if(controller == nullptr){
		return;
	}

	controller->setControl(DeviceControlType::Local);

	srand(millis());

	const bool left = rand() % 2 == 0;

	if (left){
		randomDirection = 6;
	}else {
		randomDirection = 2;
	}
}

Rotate180Action::~Rotate180Action(){
	if(controller == nullptr){
		return;
	}

	controller->setControl(DeviceControlType::Remote);
}

void Rotate180Action::loop(){
	if(controller == nullptr){
		markForDestroy();
		return;
	}

	if(readyToTransition()){
		const MotorDriveState state{ .DriveDirection = { .dir = 0, .speed = 0.0f }};
		controller->setLocally(state);
		markForDestroy();
		return;
	}

	const MotorDriveState state{ .DriveDirection = { .dir = randomDirection, .speed = 1.0f }};
	controller->setLocally(state);
}

bool Rotate180Action::readyToTransition() const{
	return millis() - startTime >= getDuration();
}

uint64_t Rotate180Action::getDuration(){
	Battery* battery = (Battery*)Services.get(Service::Battery);
	if (battery == nullptr){
		return 0;
	}

	return DurationAtFull + (1.0f - std::clamp(battery->getPerc(), (uint8_t)0, (uint8_t)100) / 100.0f) * (DurationAtEmpty - DurationAtFull);
}
