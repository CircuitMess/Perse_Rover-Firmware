#ifndef PERSE_ROVER_DEMOSTATE_H
#define PERSE_ROVER_DEMOSTATE_H

#include "Util/Events.h"
#include "Devices/MotorDriveController.h"
#include "Devices/ArmController.h"
#include "Devices/CameraController.h"
#include "Services/StateMachine.h"

class DemoState : public State {
public:
	DemoState(MotorDriveController& motors, ArmController& arm, CameraController& cam);
	virtual ~DemoState();

private:
	EventQueue evts;

	void loop() override;

	uint64_t actionTime = 0;
	int actionIndex = 0;

	struct Action {
		enum { Arm, Pinch, Cam, Move, Pause } type;
		union {
			int dir;
			int pos;
		};
		uint32_t duration;
	};

	static const Action Actions[];

	void doAction(const Action& action);
	void stopAction(const Action& action);

	MotorDriveController& motors;
	ArmController& arm;
	CameraController& cam;



};


#endif //PERSE_ROVER_DEMOSTATE_H
