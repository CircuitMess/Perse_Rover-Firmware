#include "LEDBreatheToFunction.h"
#include "Util/stdafx.h"

LEDBreatheToFunction::LEDBreatheToFunction(SingleLED& led, float targetPercent, uint32_t period) : LEDFunction(led), duration(period), startTime(millis()), targetValue(targetPercent * 0xFF) {
	startValue = led.getValue();
}

void LEDBreatheToFunction::loop(){
	const uint64_t elapsedTime = millis() - startTime;

	if(elapsedTime > duration){
		return;
	}

	const float percent = 1.0f * elapsedTime / duration;

	if(startValue > targetValue){
		led.setValue(startValue - percent * (startValue - targetValue));
	}else{
		led.setValue(startValue + percent * (targetValue - startValue));
	}
}
