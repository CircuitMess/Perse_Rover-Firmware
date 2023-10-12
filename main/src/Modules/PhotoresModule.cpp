#include "PhotoresModule.h"

PhotoresModule::PhotoresModule(ModuleBus bus) : adc(bus == ModuleBus::Left ? (gpio_num_t) A_CTRL_1 : (gpio_num_t) B_CTRL_1, ADC_ATTEN_DB_6){

}

uint8_t PhotoresModule::getLevel(){
	return (uint8_t)((adc.sample() / 4095) * 100.0f);
}
