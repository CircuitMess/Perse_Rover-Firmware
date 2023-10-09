#ifndef PERSE_ROVER_TEMPHUMMODULE_H
#define PERSE_ROVER_TEMPHUMMODULE_H

#include "Periph/I2C.h"
#include <array>

class TempHumModule {
public:
	TempHumModule(I2C& i2c);

	float getHumidity();

	float getTemp();

private:
	I2C& i2c;
	static constexpr uint8_t Addr = 0x38;

	std::array<uint8_t, 6> readData();
};


#endif //PERSE_ROVER_TEMPHUMMODULE_H
