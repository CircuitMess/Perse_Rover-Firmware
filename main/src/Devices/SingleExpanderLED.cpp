#include "SingleExpanderLED.h"

SingleExpanderLED::SingleExpanderLED(AW9523 &aw9523, uint8_t pin, uint8_t limit /*= 0xFF*/) : SingleLED(limit),
																							  aw9523(aw9523), pin(pin) {
	aw9523.pinMode(pin, AW9523::PinMode::LED);
}

void SingleExpanderLED::write(uint8_t val) {
	aw9523.dim(pin, val);
}
