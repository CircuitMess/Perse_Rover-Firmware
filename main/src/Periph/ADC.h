#ifndef PERSE_ROVER_ADC_H
#define PERSE_ROVER_ADC_H

#include <esp_adc/adc_oneshot.h>
#include <hal/gpio_types.h>

class ADC
{
public:
	explicit ADC(gpio_num_t pin, float ema_a = 1, int min = 0, int max = 0, float readingOffset = 0.0f);
	virtual ~ADC();

	float sample();
	float getValue() const;

	void resetEma();
	void setEmaA(float ema_a);

private:
	const gpio_num_t pin;
	float emaA;
	const float min;
	const float max;
	const float readingOffset;
	float value = -1.0f;
	adc_oneshot_unit_handle_t adc_handle;
};

#endif //PERSE_ROVER_ADC_H