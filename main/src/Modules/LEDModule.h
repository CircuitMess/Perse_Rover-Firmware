#ifndef PERSE_ROVER_LEDMODULE_H
#define PERSE_ROVER_LEDMODULE_H

#include "Services/Modules.h"
#include "Periph/PinOut.h"

class LEDModule {
public:
	LEDModule(ModuleBus bus);

	void on();
	void off();

private:
	PinOut pinout;
};


#endif //PERSE_ROVER_LEDMODULE_H
