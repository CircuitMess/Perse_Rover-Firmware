#include "SinglePwmLED.h"
#include <algorithm>

SinglePwmLED::SinglePwmLED(uint8_t pin, ledc_channel_t channel, uint8_t limit) : SingleLED(std::min(limit, (uint8_t)100)), pwm(pin, channel) {}

void SinglePwmLED::write(uint8_t val) {
	pwm.setDuty(val);
}
