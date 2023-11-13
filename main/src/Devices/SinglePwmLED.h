#ifndef PERSE_ROVER_SINGLEPWMLED_H
#define PERSE_ROVER_SINGLEPWMLED_H

#include "SingleLED.h"
#include "Periph/PWM.h"

class SinglePwmLED : public SingleLED {
public:
	SinglePwmLED(uint8_t pin, ledc_channel_t channel, uint8_t limit = 100);
	virtual ~SinglePwmLED() override;

protected:
	virtual void write(uint8_t val) override;

private:
	PWM pwm;
};

#endif //PERSE_ROVER_SINGLEPWMLED_H