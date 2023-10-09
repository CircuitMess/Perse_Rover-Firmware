#ifndef PERSE_ROVER_MOTIONSENSOR_H
#define PERSE_ROVER_MOTIONSENSOR_H

#include "Services/Modules.h"

class MotionSensor {
public:
	MotionSensor(ModuleBus bus);
	bool getMotion();

private:
	gpio_num_t pin;
};


#endif //PERSE_ROVER_MOTIONSENSOR_H
