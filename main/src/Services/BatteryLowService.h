#ifndef PERSE_ROVER_BATTERYLOWSERVICE_H
#define PERSE_ROVER_BATTERYLOWSERVICE_H

#include "Util/Threaded.h"
#include "Util/Events.h"
#include "Audio.h"

class BatteryLowService : private Threaded {
public:
	BatteryLowService();
	~BatteryLowService() override;

private:
	EventQueue queue;
	void loop() override;

	Audio& audio;
};

#endif //PERSE_ROVER_BATTERYLOWSERVICE_H
