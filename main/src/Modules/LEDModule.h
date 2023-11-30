#ifndef PERSE_ROVER_LEDMODULE_H
#define PERSE_ROVER_LEDMODULE_H

#include "Services/Modules.h"
#include "Periph/PinOut.h"

class LEDModule : private SleepyThreaded{
public:
	LEDModule(ModuleBus bus);
	~LEDModule() override;
private:
	PinOut pinout;
	EventQueue queue;
	ModuleBus bus;
	bool state = false;

	void sleepyLoop() override;
};


#endif //PERSE_ROVER_LEDMODULE_H
