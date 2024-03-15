#ifndef PERSE_ROVER_COMM_H
#define PERSE_ROVER_COMM_H

#include "TCPServer.h"
#include "Util/Threaded.h"
#include "Util/Events.h"
#include <CommData.h>

class Comm : private Threaded {
public:
	struct Event {
		CommType type;
		union {
			DriveDir dir;
			HeadlightsMode headlights;
			struct {
				ArmPos armPos;
				ArmPinch armPinch;
			};
			CameraRotation cameraRotation;
			uint8_t feedQuality;
			bool scanningEnable;
			bool emergency;
			bool audio;
			bool armEnabled;
			bool controllerBatteryCritical;
			ConnectionStrength connectionStrength;
		};
		uint8_t raw;
	};

	Comm();
	~Comm() override;

	void sendHeadlightsState(HeadlightsMode headlights, bool local = false);
	void sendArmPositionState(ArmPos position, bool local = false);
	void sendArmPinchState(ArmPinch pinch, bool local = false);
	void sendCameraState(CameraRotation rotation, bool local = false);
	void sendBattery(uint8_t batteryPercent);
	void sendModulePlug(ModuleType type, ModuleBus bus, bool insert);
	void sendModuleData(ModuleData data);
	void sendNoFeed(bool noFeed);

private:
	TCPServer& tcp;
	void loop() override;
	void sendPacket(const ControlPacket& packet);
	Event processPacket(const ControlPacket& packet);

	EventQueue queue;
};


#endif //PERSE_ROVER_COMM_H
