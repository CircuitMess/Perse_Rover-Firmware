#include <algorithm>
#include "SingleLED.h"
#include "Util/stdafx.h"

SingleLED::SingleLED(uint8_t limit) : limit(limit), value(0){}

void SingleLED::setValue(uint8_t val){
	if(val == value){
		return;
	}

	value = val;

	const float percent = std::clamp(1.0f * map(value, 0, 0xFF, 0, limit) / limit, 0.0f, 1.0f);

	write((uint8_t) (percent * percent * limit));
}

uint8_t SingleLED::getValue() const{
	return value;
}
