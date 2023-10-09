#ifndef PERSE_ROVER_ALTPRESS_H
#define PERSE_ROVER_ALTPRESS_H

#include "Periph/I2C.h"

class AltPressModule {
public:
	AltPressModule(I2C& i2c);

	int getPressure();
	int getAltitude();

private:
	I2C& i2c;

	enum Sensor { PRESSURE = 0x30, ALTITUDE = 0x31 };
	int readSensor(Sensor sensor);

	static constexpr uint8_t Addr = 0x76;
};


#endif //PERSE_ROVER_ALTPRESS_H
