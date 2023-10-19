#ifndef PERSE_ROVER_MOTIONSENSOR_H
#define PERSE_ROVER_MOTIONSENSOR_H

#include <atomic>
#include "Services/Modules.h"

class MotionSensor : private Threaded {
public:
	MotionSensor(ModuleBus bus, Comm& comm);
	virtual ~MotionSensor() override;

private:
	gpio_num_t pin;
	ModuleBus bus;
	Comm& comm;
	void loop() override;
	static void isr(void* arg);

	SemaphoreHandle_t sem;
	std::atomic_bool exitFlag = false;
};


#endif //PERSE_ROVER_MOTIONSENSOR_H
