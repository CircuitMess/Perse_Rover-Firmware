#ifndef PERSE_ROVER_DRIVESTATE_H
#define PERSE_ROVER_DRIVESTATE_H

#include "Services/StateMachine.h"
#include "Devices/AW9523.h"
#include "Util/Events.h"

class DriveState : public State
{
public:
	explicit DriveState();
	virtual ~DriveState() override;

protected:
	virtual void loop() override;

private:
	EventQueue queue;
};

#endif //PERSE_ROVER_DRIVESTATE_H