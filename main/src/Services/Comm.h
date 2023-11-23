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
		};
		uint8_t raw;
	};

	Comm();
	~Comm() override;

	void sendModulePlug(ModuleType type, ModuleBus bus, bool insert);
	void sendModuleData(ModuleData data);

	void sendHeadlightsState(HeadlightsMode headlights);
	void sendArmPositionState(ArmPos position);
	void sendArmPinchState(ArmPinch pinch);
	void sendCameraState(CameraRotation rotation);
	void sendBattery(uint8_t batteryPercent);

private:
	TCPServer& tcp;
	void loop() override;
	void sendPacket(const ControlPacket& packet);
	Event processPacket(const ControlPacket& packet);

	EventQueue queue;
};


#endif //PERSE_ROVER_COMM_H
