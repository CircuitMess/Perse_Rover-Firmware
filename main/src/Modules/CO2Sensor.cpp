#include "CO2Sensor.h"
#include "Util/Services.h"

CO2Sensor::CO2Sensor(ModuleBus bus, ADC& adc) : SleepyThreaded(Modules::ModuleSendInterval, "CO2", 2 * 1024),
												gpio(bus == ModuleBus::Left ? (gpio_num_t) A_CTRL_1 : (gpio_num_t) B_CTRL_1),
												adc(adc, gpio), bus(bus), comm(*((Comm*) Services.get(Service::Comm))),
												audio(*((Audio*) Services.get(Service::Audio))){
	adc_unit_t unit;
	adc_channel_t chan;
	adc_oneshot_io_to_channel(gpio, &unit, &chan);

	adc.config(chan, {
			.atten = ADC_ATTEN_DB_11,
			.bitwidth = ADC_BITWIDTH_12
	});

	start();
}

CO2Sensor::~CO2Sensor(){
	stop();
}

void CO2Sensor::sleepyLoop(){
	bool status = adc.sample() < OKThreshold;
	if(OKreading && !status){
		audio.play("/spiffs/Modules/AirBad.aac");
	}
	OKreading = status;
	const ModuleData data = {
			ModuleType::CO2, bus, { .gas = { status } }
	};

	comm.sendModuleData(data);
}
