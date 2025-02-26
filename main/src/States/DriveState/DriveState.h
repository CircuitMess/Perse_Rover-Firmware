#ifndef PERSE_ROVER_DRIVESTATE_H
#define PERSE_ROVER_DRIVESTATE_H

#include <memory>
#include <map>
#include <functional>
#include <MarkerInfo.h>
#include "Services/StateMachine.h"
#include "Devices/AW9523.h"
#include "Util/Events.h"
#include "RandSoundPlayer.h"
#include "CommData.h"
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

	RandSoundPlayer randSoundPlayer;
	static const std::unordered_set<CommType> IdleResetComms;

	Audio& audio;
	static constexpr uint32_t CamFlipPause = 1000; //[ms] - pause from start of DriveState, to prevent camera flip from accidental button presses
	uint32_t startMillis = 0;
	bool camFlip = false;
};

#endif //PERSE_ROVER_DRIVESTATE_H