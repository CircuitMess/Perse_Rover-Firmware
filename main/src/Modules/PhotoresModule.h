#ifndef PERSE_ROVER_PHOTORESMODULE_H
#define PERSE_ROVER_PHOTORESMODULE_H

#include "Services/Modules.h"
#include "Periph/ADC.h"

class PhotoresModule {
public:
	PhotoresModule(ModuleBus bus);

	uint8_t getLevel();

private:
	ADC adc;

};


#endif //PERSE_ROVER_PHOTORESMODULE_H
