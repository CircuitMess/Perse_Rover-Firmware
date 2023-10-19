#ifndef PERSE_ROVER_CO2SENSOR_H
#define PERSE_ROVER_CO2SENSOR_H

#include "Services/Modules.h"
#include "Periph/ADC.h"

class CO2Sensor : private SleepyThreaded {
public:
	CO2Sensor(ModuleBus bus, Comm& comm);
	~CO2Sensor() override;

private:
	ADC adc;
	const ModuleBus bus;
	Comm& comm;

	void sleepyLoop() override;

	static constexpr uint32_t OKThreshold = 2100; //TODO - check this value and adjust if necessary
};


#endif //PERSE_ROVER_CO2SENSOR_H
