#ifndef PERSE_ROVER_SINGLEEXPANDERLED_H
#define PERSE_ROVER_SINGLEEXPANDERLED_H

#include "SingleLED.h"
#include "AW9523.h"

class SingleExpanderLED : public SingleLED {
public:
	SingleExpanderLED(AW9523 &aw9523, uint8_t pin, uint8_t limit = 0xFF);

protected:
	virtual void write(uint8_t val) override;

private:
	AW9523 &aw9523;
	uint8_t pin;
};

#endif //PERSE_ROVER_SINGLEEXPANDERLED_H