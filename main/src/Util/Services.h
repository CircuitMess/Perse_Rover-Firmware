#ifndef BIT_FIRMWARE_SERVICES_H
#define BIT_FIRMWARE_SERVICES_H

#include <unordered_map>

enum class Service {
	TCP,
	WiFi,
	Audio,
	Comm,
	StateMachine,
	LED,
	Feed,
	Input,
	Modules,
	Battery,
	HeadLightsController,
	ArmController,
	CameraController,
	MotorDriveController,
	LowBattery,
	Settings
};

class ServiceLocator {
public:
	void set(Service service, void* ptr);
	void* get(Service service);

private:
	std::unordered_map<Service, void*> services;


};

extern ServiceLocator Services;

#endif //BIT_FIRMWARE_SERVICES_H
