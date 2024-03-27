#include "PanicAction.h"
#include "Devices/HeadlightsController.h"
#include "Devices/MotorDriveController.h"
#include "Devices/CameraController.h"
#include "Devices/ArmController.h"
#include "Util/Services.h"
#include "Util/stdafx.h"
#include "Services/Comm.h"
#include "Services/TCPServer.h"
#include "Services/LEDService.h"
#include "Services/Audio.h"
#include "Services/Feed.h"

PanicAction::PanicAction() : startTime(millis()), eventQueue(10){
	if(Feed* feed = (Feed*) Services.get(Service::Feed)){
		feed->disableScanning();
	}

	Events::listen(Facility::Comm, &eventQueue);
	Events::listen(Facility::TCP, &eventQueue);

	armController = (ArmController*) Services.get(Service::ArmController);
	if(armController != nullptr){
		armController->setControl(DeviceControlType::Local);
	}

	headlightsController = (HeadlightsController*) Services.get(Service::HeadLightsController);
	if(headlightsController != nullptr){
		headlightsController->setControl(DeviceControlType::Local);
	}

	cameraController = (CameraController*) Services.get(Service::CameraController);
	if(cameraController != nullptr){
		cameraController->setControl(DeviceControlType::Local);
	}

	motorDriveController = (MotorDriveController*) Services.get(Service::MotorDriveController);
	if(motorDriveController != nullptr){
		motorDriveController->setControl(DeviceControlType::Local);
	}

	Audio* audio = (Audio*) Services.get(Service::Audio);
	if (audio != nullptr) {
		audio->stop();
		audio->play("/spiffs/Systems/PanicOn.aac", true);
	}
}

PanicAction::~PanicAction(){
	Events::unlisten(&eventQueue);

	if(armController != nullptr){
		armController->setControl(DeviceControlType::Remote);
	}

	if(headlightsController != nullptr){
		headlightsController->setLocally({});
		headlightsController->setControl(DeviceControlType::Remote);
	}

	if(cameraController != nullptr){
		cameraController->setControl(DeviceControlType::Remote);
	}

	if(motorDriveController != nullptr){
		motorDriveController->setControl(DeviceControlType::Remote);
	}

	LEDService* led = (LEDService*) Services.get(Service::LED);
	if (led == nullptr){
		return;
	}

	Audio* audio = (Audio*) Services.get(Service::Audio);
	if (audio != nullptr) {
		audio->stop();
		audio->play("/spiffs/Systems/PanicOff.aac", true);
	}

	led->off(LED::StatusYellow);
	led->off(LED::StatusRed);
	led->breathe(LED::Rear);
}

void PanicAction::loop(){
	for(Event event{}; eventQueue.get(event, 0); ){
		if(event.facility == Facility::Comm){
			const Comm::Event* commEvent = (Comm::Event*) event.data;
			if(commEvent->type == CommType::Emergency && !commEvent->emergency){
				markForDestroy();
			}

			free(event.data);
		}else if(event.facility == Facility::TCP){
			const TCPServer::Event* tcpEvent = (TCPServer::Event*) event.data;
			if (tcpEvent->status == TCPServer::Event::Status::Disconnected){
				markForDestroy();
			}
		}
	}

	const uint64_t deltaTime = millis() - startTime;

	const uint32_t iteration = deltaTime / DelayBetweenMovements;

	if(iteration == 0){
		armController->setLocally({.Position = 100, .Pinch = 100});
	}else if(iteration == 1){
		cameraController->setLocally({.Rotation = 0});
	}else if(iteration == 2){
		cameraController->setLocally({.Rotation = 100});
	}else if(iteration == 3){
		cameraController->setLocally({.Rotation = 50});
	}else if(iteration >= 4 && iteration <= 6){
		motorDriveController->setLocally({.DriveDirection = {.dir = 4, .speed = 1.0f}});
	}else if(iteration == 7){
		motorDriveController->setLocally({.DriveDirection = {.dir = 0, .speed = 0.0f}});



		LEDService* led = (LEDService*) Services.get(Service::LED);
		if (led == nullptr){
			return;
		}

		led->blink(LED::StatusRed, 0, 2 * DelayBetweenMovements);
		led->blink(LED::Rear, 0, DelayBetweenMovements);
		led->blink(LED::HeadlightLeft, 0, DelayBetweenMovements);
		led->blink(LED::HeadlightsRight, 0, DelayBetweenMovements);
	}else if(iteration == 8){
		LEDService* led = (LEDService*) Services.get(Service::LED);
		if (led == nullptr){
			return;
		}

		led->blink(LED::StatusYellow, 0, 2 * DelayBetweenMovements);
	}
}

bool PanicAction::readyToTransition() const{
	return false;
}
