#include "PhotoresModule.h"

PhotoresModule::PhotoresModule(ModuleBus bus, Comm& comm, ADC& adc) : SleepyThreaded(Modules::ModuleSendInterval, "Photores", 2 * 1024),
																	  gpio(bus == ModuleBus::Left ? (gpio_num_t) A_CTRL_1 : (gpio_num_t) B_CTRL_1), comm(comm),
																	  bus(bus), adc(adc, gpio){
	adc_unit_t unit;
	adc_channel_t chan;
	adc_oneshot_io_to_channel(gpio, &unit, &chan);

	adc.config(chan, {
			.atten = ADC_ATTEN_DB_6,
			.bitwidth = ADC_BITWIDTH_12
	});

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
