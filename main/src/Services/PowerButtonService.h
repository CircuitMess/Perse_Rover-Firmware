#ifndef PERSE_ROVER_POWERBUTTONSERVICE_H
#define PERSE_ROVER_POWERBUTTONSERVICE_H

#include "Util/Threaded.h"
#include "Util/Events.h"

/**
 * Turns a hold of the power button into an orderly power off. Short presses are left to whatever
 * state is running (DriveState uses them to flip the camera).
 */
class PowerButtonService : private Threaded {
public:
	PowerButtonService();
	~PowerButtonService() override;

private:
	EventQueue queue;

	void loop() override;
};

#endif //PERSE_ROVER_POWERBUTTONSERVICE_H
