#ifndef PERSE_ROVER_CO2SENSOR_H
#define PERSE_ROVER_CO2SENSOR_H

#include "Services/Modules.h"
#include "Periph/ADC.h"

class CO2Sensor {
public:
	CO2Sensor(ModuleBus bus);

	bool getStatus();

private:
	ADC adc;

	static constexpr uint32_t OKThreshold = 2100;
};


#endif //PERSE_ROVER_CO2SENSOR_H
