#ifndef PERSE_ROVER_TURNLEFTGOAHEADACTION_H
#define PERSE_ROVER_TURNLEFTGOAHEADACTION_H

#include <cstdint>
#include "PlayAudioAction.h"

class TurnLeftGoAheadAction : public PlayAudioAction {
public:
	TurnLeftGoAheadAction();
	virtual ~TurnLeftGoAheadAction() override;

protected:
	inline virtual constexpr const char* getFile() const override {
		return "/spiffs/Markers/LeftForward.aac";
	}

	virtual void loop() override;
	virtual bool readyToTransition() const override;

private:
	constexpr static uint64_t TurnDurationAtFull = 1100;
	constexpr static uint64_t TurnDurationAtEmpty = 1500;
	static constexpr uint64_t ForwardDuration = 1750;
	class MotorDriveController* controller = nullptr;
	uint64_t startTime = 0;
};

#endif //PERSE_ROVER_TURNLEFTGOAHEADACTION_H