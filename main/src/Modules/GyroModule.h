#ifndef PERSE_ROVER_GYROMODULE_H
#define PERSE_ROVER_GYROMODULE_H

#include <glm.hpp>
#include "Periph/I2C.h"

class GyroModule {
public:
	GyroModule(I2C& i2c);

	glm::vec3 getAccelerometer() const;
	int8_t getTemperature() const;

private:
	I2C& i2c;
	static constexpr uint8_t Addr = 0x18;
};


#endif //PERSE_ROVER_GYROMODULE_H
