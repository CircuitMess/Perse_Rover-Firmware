#include "PhotoresModule.h"

PhotoresModule::PhotoresModule(ModuleBus bus, Comm& comm) : SleepyThreaded(Modules::ModuleSendInterval, "Photores", 2 * 1024), comm(comm), bus(bus),
															adc(bus == ModuleBus::Left ? (gpio_num_t) A_CTRL_1 : (gpio_num_t) B_CTRL_1, ADC_ATTEN_DB_6){
	start();
}

PhotoresModule::~PhotoresModule(){
	stop();
}

void PhotoresModule::sleepyLoop(){
	auto level = getLevel();
	ModuleData data = {
			ModuleType::PhotoRes, bus, { .photoRes = { level } }
	};
	comm.sendModuleData(data);
}

uint8_t PhotoresModule::getLevel(){
	return (uint8_t) ((adc.sample() / 4095) * 100.0f);
}
