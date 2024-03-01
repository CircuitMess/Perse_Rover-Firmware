#ifndef PERSE_ROVER_DRIVESTATE_H
#define PERSE_ROVER_DRIVESTATE_H

#include <memory>
#include <map>
#include <functional>
#include <MarkerInfo.h>
#include "Services/StateMachine.h"
#include "Devices/AW9523.h"
#include "Util/Events.h"
#include "Services/Audio.h"

class DriveState : public State {
public:
	explicit DriveState();
	virtual ~DriveState() override;

protected:
	virtual void loop() override;

private:
	static const std::map<MarkerAction, std::function<std::unique_ptr<class Action>(void)>> actionMappings;
	EventQueue queue;
	std::unique_ptr<Action> activeAction;

	Audio& audio;
};

#endif //PERSE_ROVER_DRIVESTATE_H