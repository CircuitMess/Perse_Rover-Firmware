#include "CO2Sensor.h"

CO2Sensor::CO2Sensor(ModuleBus bus, Comm& comm) : SleepyThreaded(Modules::ModuleSendInterval, "CO2", 2 * 1024),
												  adc(bus == ModuleBus::Left ? (gpio_num_t) A_CTRL_1 : (gpio_num_t) B_CTRL_1, ADC_ATTEN_DB_11), bus(bus),
												  comm(comm){
	start();
}

CO2Sensor::~CO2Sensor(){
	stop();
}

void CO2Sensor::sleepyLoop(){
	bool status = adc.sample() > OKThreshold;
	ModuleData data = {
			ModuleType::CO2, bus, { .gas = { status } }
	};
	comm.sendModuleData(data);
}
