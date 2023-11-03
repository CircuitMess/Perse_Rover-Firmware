#ifndef PERSE_ROVER_PHOTORESMODULE_H
#define PERSE_ROVER_PHOTORESMODULE_H

#include "Services/Modules.h"
#include "Services/ADCReader.h"

class PhotoresModule : private SleepyThreaded {
public:
	PhotoresModule(ModuleBus bus, ADC& adc);
	~PhotoresModule() override;

private:
	gpio_num_t gpio;
	Comm& comm;
	ModuleBus bus;

	void sleepyLoop() override;
	ADCReader adc;

	uint8_t getLevel();
};


#endif //PERSE_ROVER_PHOTORESMODULE_H
