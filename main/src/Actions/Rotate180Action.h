#ifndef PERSE_ROVER_ROTATE180ACTION_H
#define PERSE_ROVER_ROTATE180ACTION_H

#include <cstdint>
#include "PlayAudioAction.h"

class Rotate180Action : public PlayAudioAction {
public:
	Rotate180Action();
	virtual ~Rotate180Action() override;

protected:
	inline virtual constexpr const char* getFile() const override {
		return "/spiffs/Markers/Rotate.aac";
	}

	virtual void loop() override;
	virtual bool readyToTransition() const override;

private:
	constexpr static uint64_t DurationAtFull = 2200;
	constexpr static uint64_t DurationAtEmpty = 3000;
	uint64_t startTime = 0;
	uint8_t randomDirection;
	class MotorDriveController* controller = nullptr;

private:
	static uint64_t getDuration();
};

#endif //PERSE_ROVER_ROTATE180ACTION_H