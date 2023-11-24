#ifndef PERSE_ROVER_GOTOWARDSACTION_H
#define PERSE_ROVER_GOTOWARDSACTION_H

#include "Action.h"

class GoTowardsAction : public Action {
public:
	GoTowardsAction();
	virtual ~GoTowardsAction();
protected:
	virtual void loop() override;

private:
	class MotorDriveController* controller = nullptr;
};

#endif //PERSE_ROVER_GOTOWARDSACTION_H