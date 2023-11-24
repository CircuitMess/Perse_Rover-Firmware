#include "MotorDriveController.h"
#include <esp_log.h>
#include "Services/Comm.h"
#include "Devices/Motors.h"
#include "Services/LEDService.h"
#include "Util/Services.h"

static const char* const TAG = "MotorDriveController";

MotorDriveController::MotorDriveController() : DeviceController("MotorDriveController"), motorControl(new MotorControl(std::array<ledc_channel_t, 2>({LEDC_CHANNEL_1, LEDC_CHANNEL_2}))) {
	setControl(DeviceControlType::Local);
	setLocally(MotorDriveState{});
	setControl(DeviceControlType::Remote);
}

MotorDriveController::~MotorDriveController() {
	delete motorControl;
}

void MotorDriveController::write(const MotorDriveState &state) {
	if (motorControl == nullptr) {
		ESP_LOGW(TAG, "Motor drive controller has motor control = nullptr.");
		return;
	}

	const uint8_t dir = state.DriveDirection.dir;
	const float speed = state.DriveDirection.speed;

	float leftSpeed = 0.0f;
	float rightSpeed = 0.0f;

	if(dir == 0){
		leftSpeed = rightSpeed = 100;
	}
	else if(dir == 1){
		leftSpeed = 100;
		rightSpeed = 30;
	}
	else if(dir == 2){
		leftSpeed = 100;
		rightSpeed = -100;
	}
	else if(dir == 3){
		leftSpeed = -100;
		rightSpeed = -30;
	}
	else if(dir == 4){
		leftSpeed = rightSpeed = -100;
	}
	else if(dir == 5){
		leftSpeed = -30;
		rightSpeed = -100;
	}
	else if(dir == 6){
		leftSpeed = -100;
		rightSpeed = 100;
	}
	else if(dir == 7){
		leftSpeed = 30;
		rightSpeed = 100;
	}

	leftSpeed = std::clamp(leftSpeed * speed, -100.0f, 100.0f);
	rightSpeed = std::clamp(rightSpeed * speed, -100.0f, 100.0f);

	motorControl->setLeft(leftSpeed);
	motorControl->setRight(rightSpeed);

	LEDService* led = (LEDService*) Services.get(Service::LED);
	if(led == nullptr){
		return;
	}

	if(abs(leftSpeed) >= 50.0f){
		led->on(LED::MotorLeft);
	}else{
		led->off(LED::MotorLeft);
	}

	if(abs(rightSpeed) >= 50.0f){
		led->on(LED::MotorRight);
	}else{
		led->off(LED::MotorRight);
	}
}

MotorDriveState MotorDriveController::getDefaultState() const {
	return MotorDriveState{};
}

void MotorDriveController::sendState(const MotorDriveState &state) const {
	// No need for Motor Drive to send state back at this time
}

void MotorDriveController::processEvent(const Event &event) {
	auto* commEvent = (Comm::Event*)event.data;
	if (commEvent == nullptr){
		return;
	}

	if (commEvent->type != CommType::DriveDir) {
		return;
	}

	MotorDriveState state = {
			.DriveDirection = commEvent->dir
	};

	setRemotely(state);
}