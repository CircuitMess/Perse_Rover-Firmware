#ifndef PERSE_ROVER_PHOTORESMODULE_H
#define PERSE_ROVER_PHOTORESMODULE_H

#include "Services/Modules.h"
#include "Periph/ADC.h"

class PhotoresModule : private SleepyThreaded {
public:
	PhotoresModule(ModuleBus bus, Comm& comm);
	~PhotoresModule() override;

private:
	Comm& comm;
	ModuleBus bus;

	void sleepyLoop() override;
	ADC adc;

	uint8_t getLevel();
};


#endif //PERSE_ROVER_PHOTORESMODULE_H
