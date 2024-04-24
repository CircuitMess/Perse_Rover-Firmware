#ifndef PERSE_ROVER_GOTOWARDSACTION_H
#define PERSE_ROVER_GOTOWARDSACTION_H

#include "PlayAudioAction.h"

class GoTowardsAction : public PlayAudioAction {
public:
	GoTowardsAction();
	virtual ~GoTowardsAction();
protected:
	inline virtual constexpr const char* getFile() const override {
		return "/spiffs/Markers/Advancing.aac";
	}

	virtual void loop() override;

private:
	class MotorDriveController* controller = nullptr;
};

#endif //PERSE_ROVER_GOTOWARDSACTION_H