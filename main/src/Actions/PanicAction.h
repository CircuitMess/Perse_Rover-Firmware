#ifndef PERSE_ROVER_PANICACTION_H
#define PERSE_ROVER_PANICACTION_H

#include <cstdint>
#include "Action.h"
#include "Util/Events.h"

class PanicAction : public Action {
public:
	PanicAction();
	virtual ~PanicAction() override;

	virtual void loop() override;
	virtual bool readyToTransition() const override;

private:
	static constexpr uint64_t DelayBetweenMovements = 1000;
	class ArmController* armController = nullptr;
	class CameraController* cameraController = nullptr;
	class HeadlightsController* headlightsController = nullptr;
	class MotorDriveController* motorDriveController = nullptr;

	uint64_t startTime = 0;
	EventQueue eventQueue;
};

#endif //PERSE_ROVER_PANICACTION_H