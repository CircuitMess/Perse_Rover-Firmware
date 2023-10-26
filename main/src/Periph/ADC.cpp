#include "ADC.h"
#include <algorithm>
#include <esp_adc/adc_continuous.h>

static const char* TAG = "ADC";

ADC::ADC(gpio_num_t pin, float ema_a/* = 1*/, int min/* = 0*/, int max/* = 0*/, float readingOffset/* = 0.0*/) :
			pin(pin), emaA(ema_a), min(min), max(max), readingOffset(readingOffset), unit(ADC_UNIT_1), channel(ADC_CHANNEL_0) {
	ESP_ERROR_CHECK(adc_continuous_io_to_channel(pin, &unit, &channel));

	adc_oneshot_unit_init_cfg_t config = {
			.unit_id = unit,
			.ulp_mode = ADC_ULP_MODE_DISABLE
	};

	ESP_ERROR_CHECK(adc_oneshot_new_unit(&config, &adc_handle));

	adc_oneshot_chan_cfg_t channelConfig = {
			.atten = ADC_ATTEN_DB_11,
			.bitwidth = ADC_BITWIDTH_12
	};

	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, channel, &channelConfig));

	sample();
}

ADC::~ADC() {
	ESP_ERROR_CHECK(adc_oneshot_del_unit(adc_handle));
}

float ADC::sample() {
	int raw = 0;
	ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, channel, &raw));

	if (value == -1 || emaA == 1) {
		value = raw;
	}
	else {
		value = value * (1.0f - emaA) + emaA * raw;
	}

	return getValue();
}

float ADC::getValue() const {
	if (max == 0 && min == 0) {
		return value + readingOffset;
	}

	float minimum = min;
	float maximum = max;

	if (min > max) {
		std::swap(minimum, maximum);
	}

	float val = std::clamp(value, minimum, maximum);
	val = (val - minimum) / (maximum - minimum);
	val = std::clamp(val * 100.0f, 0.0f, 100.0f);

	if (min > max) {
		val = 100.0f - val;
	}

	return val;
}

void ADC::resetEma() {
	value = -1;
	sample();
}

void ADC::setEmaA(float ema_a) {
	emaA = ema_a;
}
