#include "CameraController.h"
#include <esp_log.h>
#include "Servo.h"
#include "Pins.hpp"
#include "Services/Comm.h"
#include "Util/Services.h"
#include "Util/stdafx.h"

static const char* const TAG = "HeadlightsController";

CameraController::CameraController() : DeviceController("CameraController"), ease("Cam", 1, 5, [this](int32_t val){ cameraServo->setValue(val); }){
	cameraServo = new Servo((gpio_num_t)SERVO_3_PWM, 0);

	if (cameraServo == nullptr) {
		ESP_LOGW(TAG, "Camera controller created with invalid servo device.");
		return;
	}

	cameraServo->enable();

	setControl(DeviceControlType::Local);
	setLocally(CameraState{});
	setControl(DeviceControlType::Remote);
}

CameraController::~CameraController() {
	if (cameraServo == nullptr) {
		return;
	}

	cameraServo->disable();
	delete cameraServo;
}

void CameraController::write(const CameraState &state) {
	if (cameraServo == nullptr) {
		ESP_LOGW(TAG, "Write attempted with invalid camera servo device.");
		return;
	}

	uint8_t value = map(std::clamp(state.Rotation, (uint8_t)0, (uint8_t)100), 0, 100, rotationLimits.x, rotationLimits.y);
	ease.set(value);
}

CameraState CameraController::getDefaultState() const {
	return CameraState{};
}

void CameraController::sendState(const CameraState &state, bool local) const {
	auto comm = (Comm*)Services.get(Service::Comm);
	if (comm == nullptr){
		return;
	}

	comm->sendCameraState(state.Rotation, local);
}

void CameraController::processEvent(const Event &event) {
	auto* commEvent = (Comm::Event*)event.data;
	if (commEvent == nullptr) {
		return;
	}

	if (commEvent->type != CommType::CameraRotation) {
		return;
	}

	CameraState state = {
			.Rotation = commEvent->cameraRotation
	};

	setRemotely(state);
}
