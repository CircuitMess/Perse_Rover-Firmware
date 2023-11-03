#include "ArmController.h"
#include <esp_log.h>
#include "Devices/Servo.h"
#include "Pins.hpp"
#include "Services/Comm.h"
#include "Util/Services.h"
#include "Util/stdafx.h"

static const char* const TAG = "HeadlightsController";

ArmController::ArmController() : DeviceController("ArmController") {
	positionServo = new Servo((gpio_num_t)SERVO_1_PWM, 0);
	pinchServo = new Servo((gpio_num_t)SERVO_2_PWM, 0);

	if (positionServo == nullptr || pinchServo == nullptr) {
		ESP_LOGW(TAG, "Arm controller initialized with invalid servo.");
		return;
	}

	positionServo->enable();
	pinchServo->enable();

	setControl(DeviceControlType::Local);
	setLocally({50, 50});
	setControl(DeviceControlType::Remote);
}

ArmController::~ArmController() {
	if (positionServo != nullptr) {
		positionServo->disable();
		delete positionServo;
	}

	if (pinchServo != nullptr) {
		pinchServo->disable();
		delete pinchServo;
	}
}

void ArmController::write(const ArmState& state) {
	if (pinchServo == nullptr || positionServo == nullptr) {
		ESP_LOGW(TAG, "Arm controller attempted to write to an invalid servo device.");
		return;
	}

	if (state.Pinch >= 0) {
		const uint8_t servoValue = map(std::clamp(state.Pinch, (int8_t)0, (int8_t)100), 0, 100, pinchLimits.x, pinchLimits.y);
		pinchServo->setValue(servoValue);
	}

	if (state.Position >= 0) {
		const uint8_t servoValue = map(std::clamp(state.Position, (int8_t)0, (int8_t)100), 0, 100, positionLimits.x, positionLimits.y);
		positionServo->setValue(servoValue);
	}
}

ArmState ArmController::getDefaultState() const {
	return ArmState{50, 50};
}

void ArmController::sendState(const ArmState& state) const {
	auto comm = (Comm*)Services.get(Service::Comm);
	if (comm == nullptr) {
		return;
	}

	if(state.Position >= 0){
		comm->sendArmPositionState(state.Position);
	}

	if(state.Pinch >= 0){
		comm->sendArmPinchState(state.Pinch);
	}
}

void ArmController::processEvent(const Event& event) {
	auto* commEvent = (Comm::Event*)event.data;
	if (commEvent == nullptr) {
		return;
	}

	if (commEvent->type != CommType::ArmPinch && commEvent->type != CommType::ArmPosition) {
		return;
	}

	ArmState state = {
			.Position = commEvent->armPos,
			.Pinch = commEvent->armPinch
	};

	setRemotely(state);
}
