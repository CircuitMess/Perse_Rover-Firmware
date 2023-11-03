#ifndef PERSE_ROVER_LEDMODULE_H
#define PERSE_ROVER_LEDMODULE_H

#include "Services/Modules.h"
#include "Periph/PinOut.h"

class LEDModule : private Threaded{
public:
	LEDModule(ModuleBus bus);
	~LEDModule() override;
private:
	PinOut pinout;
	EventQueue queue;

	void loop() override;
};


#endif //PERSE_ROVER_LEDMODULE_H
