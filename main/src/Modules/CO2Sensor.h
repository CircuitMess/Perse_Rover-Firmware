#ifndef PERSE_ROVER_CO2SENSOR_H
#define PERSE_ROVER_CO2SENSOR_H

#include "Services/Modules.h"
#include "Services/ADCReader.h"

class CO2Sensor : private SleepyThreaded {
public:
	CO2Sensor(ModuleBus bus, Comm& comm, ADC& adc);
	~CO2Sensor() override;

private:
	gpio_num_t gpio;
	ADCReader adc;
	const ModuleBus bus;
	Comm& comm;

	void sleepyLoop() override;

	static constexpr uint32_t OKThreshold = 2100; //TODO - check this value and adjust if necessary
};


#endif //PERSE_ROVER_CO2SENSOR_H
