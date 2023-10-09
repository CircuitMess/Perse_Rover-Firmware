#include "ADC.h"
#include <esp_log.h>
#include <algorithm>

static const char* TAG = "ADC";
const std::map<gpio_num_t, ADC::ADCConfig> ADC::gpioChannels = {
		{ GPIO_NUM_1,  { ADC_UNIT_1, ADC_CHANNEL_0 } },
		{ GPIO_NUM_2,  { ADC_UNIT_1, ADC_CHANNEL_1 } },
		{ GPIO_NUM_3,  { ADC_UNIT_1, ADC_CHANNEL_2 } },
		{ GPIO_NUM_4,  { ADC_UNIT_1, ADC_CHANNEL_3 } },
		{ GPIO_NUM_5,  { ADC_UNIT_1, ADC_CHANNEL_4 } },
		{ GPIO_NUM_6,  { ADC_UNIT_1, ADC_CHANNEL_5 } },
		{ GPIO_NUM_7,  { ADC_UNIT_1, ADC_CHANNEL_6 } },
		{ GPIO_NUM_8,  { ADC_UNIT_1, ADC_CHANNEL_7 } },
		{ GPIO_NUM_9,  { ADC_UNIT_1, ADC_CHANNEL_8 } },
		{ GPIO_NUM_10, { ADC_UNIT_1, ADC_CHANNEL_9 } },
		{ GPIO_NUM_11, { ADC_UNIT_2, ADC_CHANNEL_0 } },
		{ GPIO_NUM_12, { ADC_UNIT_2, ADC_CHANNEL_1 } },
		{ GPIO_NUM_13, { ADC_UNIT_2, ADC_CHANNEL_2 } },
		{ GPIO_NUM_14, { ADC_UNIT_2, ADC_CHANNEL_3 } },
		{ GPIO_NUM_15, { ADC_UNIT_2, ADC_CHANNEL_4 } },
		{ GPIO_NUM_16, { ADC_UNIT_2, ADC_CHANNEL_5 } },
		{ GPIO_NUM_17, { ADC_UNIT_2, ADC_CHANNEL_6 } },
		{ GPIO_NUM_18, { ADC_UNIT_2, ADC_CHANNEL_7 } },
		{ GPIO_NUM_19, { ADC_UNIT_2, ADC_CHANNEL_8 } },
		{ GPIO_NUM_20, { ADC_UNIT_2, ADC_CHANNEL_9 } }
};

ADC::ADC(gpio_num_t pin, adc_atten_t atten, float ema_a, int min, int max, int readingOffset) : pin(pin), ema_a(ema_a), min(min), max(max),
																								offset(readingOffset){
	if(!gpioChannels.contains(pin)){
		ESP_LOGE(TAG, "Only GPIOs 1-20 are supported for ADC");
		valid = false;
		return;
	}

	const auto& pinConfig = gpioChannels.at(pin);


	//-------------ADC1 Init---------------//
	adc_oneshot_unit_init_cfg_t init_config1 = {
			.unit_id = pinConfig.unit,
	};
	ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

	//-------------ADC1 Config---------------//
	adc_oneshot_chan_cfg_t config = {
			.atten = atten,
			.bitwidth = ADC_BITWIDTH_12
	};
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, pinConfig.channel, &config));

	sample();
}


ADC::~ADC(){
	adc_oneshot_del_unit(adc1_handle);
}

void ADC::setEmaA(float emaA){
	ema_a = emaA;
}

void ADC::resetEma(){
	val = -1;
	sample();
}

float ADC::sample(){
	if(!valid){
		return 0;
	}

	int reading = 0;
	ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, gpioChannels.at(pin).channel, &reading));

	if(val == -1 || ema_a == 1){
		val = (float) reading;
	}else{
		val = val * (1.0f - ema_a) + ema_a * (float) reading;
	}

	return getVal();
}

float ADC::getVal() const{
	if(!valid){
		return 0;
	}

	if(max == 0 && min == 0){
		return val + offset;
	}

	float min = this->min;
	float max = this->max;
	bool inverted = min > max;
	if(inverted){
		std::swap(min, max);
	}

	float val = std::clamp(this->val + offset, min, max);
	val = (val - min) / (max - min);
	val = std::clamp(val * 100.0f, 0.0f, 100.0f);

	if(inverted){
		val = 100.0f - val;
	}

	return val;
}


