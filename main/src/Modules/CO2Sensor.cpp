#include "CO2Sensor.h"

CO2Sensor::CO2Sensor(ModuleBus bus) : adc(bus == ModuleBus::Bus_A ? (gpio_num_t) A_CTRL_1 : (gpio_num_t) B_CTRL_1, ADC_ATTEN_DB_11){

}

bool CO2Sensor::getStatus(){
	return adc.sample() > OKThreshold;

}
