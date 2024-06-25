#include "ADCReader.h"
#include <algorithm>
#include <math.h>

ADCReader::ADCReader(ADC& adc, gpio_num_t pin, float ema_a, int min, int max, float readingOffset, float readingFactor1, float readingFactor2) : adc(adc), emaA(ema_a), min(min), max(max), readingOffset(readingOffset), readingFactor1(readingFactor1), readingFactor2(readingFactor2){
	adc_unit_t unit;
	ESP_ERROR_CHECK(adc_oneshot_io_to_channel(pin, &unit, &chan));
	assert(unit == adc.getUnit());
}

float rawValue = 0;
float ADCReader::sample(){
	int raw = 0;
	if(adc.read(chan, raw) != ESP_OK){
		return getValue();
	}

	if(value == -1 || emaA == 1){
		value = raw;
	}else{
		value = value * (1.0f - emaA) + emaA * raw;
	}

	return getValue();
}

float ADCReader::getValue() const{
	const float adjusted = readingOffset + value * readingFactor1 + std::pow(value, 2.0f) * readingFactor2;

	if(max == 0 && min == 0){
		return adjusted;
	}

	float minimum = min;
	float maximum = max;

	if(min > max){
		std::swap(minimum, maximum);
	}

	float val = std::clamp(adjusted, minimum, maximum);
	val = (val - minimum) / (maximum - minimum);
	val = std::clamp(val * 100.0f, 0.0f, 100.0f);

	if(min > max){
		val = 100.0f - val;
	}

	return val;
}

void ADCReader::resetEma(){
	value = -1;
	sample();
}

void ADCReader::setEmaA(float ema_a){
	emaA = ema_a;
}

