#ifndef PERSE_ROVER_LEDBLINKFUNCTION_H
#define PERSE_ROVER_LEDBLINKFUNCTION_H

#include "LEDFunction.h"

class LEDBlinkFunction : public LEDFunction {
public:
	LEDBlinkFunction(SingleLED& led, uint32_t count, uint32_t period);

	virtual ~LEDBlinkFunction() override;

protected:
	virtual void loop() override;

private:
	uint32_t count;
	uint32_t period;
	uint64_t startTime;
	uint32_t elapsedCount = 1;
};

#endif //PERSE_ROVER_LEDBLINKFUNCTION_H