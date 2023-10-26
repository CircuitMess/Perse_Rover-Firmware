#ifndef PERSE_ROVER_PAIRSTATE_H
#define PERSE_ROVER_PAIRSTATE_H

#include "Services/StateMachine.h"
#include "Devices/AW9523.h"
#include "Util/Events.h"
#include "Services/PairService.h"

class PairState : public State
{
public:
	explicit PairState();
	virtual ~PairState() override;

protected:
	virtual void loop() override;

private:
	EventQueue queue;
	PairService* pairService = nullptr;
};

#endif //PERSE_ROVER_PAIRSTATE_H