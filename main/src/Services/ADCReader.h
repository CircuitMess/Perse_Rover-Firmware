#ifndef PERSE_MISSIONCTRL_ADCREADER_H
#define PERSE_MISSIONCTRL_ADCREADER_H

#include "Periph/ADC.h"
#include <hal/gpio_types.h>

class ADCReader {
public:
	ADCReader(ADC& adc, gpio_num_t pin, float ema_a = 1, int min = 0, int max = 0, float readingOffset = 0.0f, float readingFactor1 = 1.0f, float readingFactor2 = 0.0f);

	float sample();
	float getValue() const;

	void resetEma();
	void setEmaA(float ema_a);

private:
	ADC& adc;
	adc_channel_t chan;

	float emaA;
	const float min;
	const float max;
	const float readingOffset;
	const float readingFactor1;
	const float readingFactor2;

	float value = -1.0f;

};


#endif //PERSE_MISSIONCTRL_ADCREADER_H
