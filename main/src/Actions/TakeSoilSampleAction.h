#ifndef PERSE_ROVER_TAKESOILSAMPLEACTION_H
#define PERSE_ROVER_TAKESOILSAMPLEACTION_H

#include <cstdint>
#include <CommData.h>
#include "PlayAudioAction.h"

class TakeSoilSampleAction : public PlayAudioAction {
public:
	TakeSoilSampleAction();
	virtual ~TakeSoilSampleAction() override;

protected:
	inline virtual constexpr const char* getFile() const override {
		return "/spiffs/Markers/TakingSample.aac";
	}

	virtual void loop() override;
	virtual bool readyToTransition() const override;

private:
	static constexpr uint64_t ArmMoveDuration = 1000;
	static constexpr uint64_t PinchDuration = 200;
	static constexpr ArmPos TargetArmPosition = 32;
	static constexpr ArmPinch TargetArmPinch = 0;
	static constexpr ArmPos StartingArmPosition = 90;
	static constexpr ArmPinch StartingArmPinch = 95;
	uint64_t startTime = 0;
	class ArmController* controller = nullptr;
};

#endif //PERSE_ROVER_TAKESOILSAMPLEACTION_H