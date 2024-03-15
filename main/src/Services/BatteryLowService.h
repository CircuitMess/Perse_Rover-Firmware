#ifndef PERSE_ROVER_BATTERYLOWSERVICE_H
#define PERSE_ROVER_BATTERYLOWSERVICE_H

#include "Util/Threaded.h"
#include "Util/Events.h"

class BatteryLowService : private Threaded {
public:
	BatteryLowService();
	~BatteryLowService() override;

private:
	EventQueue queue;
	void loop() override;
};

#endif //PERSE_ROVER_BATTERYLOWSERVICE_H
