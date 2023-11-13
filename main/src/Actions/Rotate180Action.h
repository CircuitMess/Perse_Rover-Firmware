#ifndef PERSE_ROVER_ROTATE180ACTION_H
#define PERSE_ROVER_ROTATE180ACTION_H

#include <cstdint>
#include "Action.h"

class Rotate180Action : public Action {
public:
	Rotate180Action();
	virtual ~Rotate180Action() override;

protected:
	virtual void loop() override;
	virtual bool readyToTransition() const override;

private:
	constexpr static uint64_t DurationAtFull = 3150;
	constexpr static uint64_t DurationAtEmpty = 4500;
	uint64_t startTime = 0;
	uint8_t randomDirection;
	class MotorDriveController* controller = nullptr;

private:
	static uint64_t getDuration();
};

#endif //PERSE_ROVER_ROTATE180ACTION_H