#ifndef PERSE_ROVER_PINOUT_H
#define PERSE_ROVER_PINOUT_H

#include <hal/gpio_types.h>

class PinOut {
public:
	PinOut(gpio_num_t pin, bool inverted = false);
	PinOut(int pin, bool inverted = false);

	void on();
	void off();
	void set(bool state);

private:
	gpio_num_t pin;
	bool inverted;

};


#endif //PERSE_ROVER_PINOUT_H
