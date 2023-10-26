#ifndef PERSE_ROVER_TCA9555_H
#define PERSE_ROVER_TCA9555_H


#include "Periph/I2C.h"

class TCA9555 {
public:
	TCA9555(I2C& i2C, uint8_t addr = 0x20);

	void reset();

	enum PinMode { IN, OUT };

	void pinMode(uint8_t pin, PinMode mode);

	bool read(uint8_t pin);
	uint16_t readAll();

	void write(uint8_t pin, bool state);

private:
	I2C& i2c;
	const uint8_t Addr;

	struct Regs {
		uint8_t dir[2] = { 0xff, 0xff };
		uint8_t output[2] = { 0x00, 0x00 };
	} regs;

};


#endif //PERSE_ROVER_TCA9555_H
