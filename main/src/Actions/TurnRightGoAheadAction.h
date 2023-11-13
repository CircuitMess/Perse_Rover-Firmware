#ifndef PERSE_ROVER_TURNRIGHTGOAHEADACTION_H
#define PERSE_ROVER_TURNRIGHTGOAHEADACTION_H

#include <cstdint>
#include "Action.h"

class TurnRightGoAheadAction : public Action {
public:
	TurnRightGoAheadAction();
	virtual ~TurnRightGoAheadAction() override;

protected:
	virtual void loop() override;
	virtual bool readyToTransition() const override;

private:
	constexpr static uint64_t TurnDurationAtFull = 1575;
	constexpr static uint64_t TurnDurationAtEmpty = 2250;
	static constexpr uint64_t ForwardDuration = 3500;
	class MotorDriveController* controller = nullptr;
	uint64_t startTime = 0;
};

#endif //PERSE_ROVER_TURNRIGHTGOAHEADACTION_H