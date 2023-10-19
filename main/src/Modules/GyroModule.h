#ifndef PERSE_ROVER_GYROMODULE_H
#define PERSE_ROVER_GYROMODULE_H

#include <glm.hpp>
#include "Periph/I2C.h"
#include "Services/Comm.h"

class GyroModule : private SleepyThreaded {
public:
	GyroModule(I2C& i2c, ModuleBus bus, Comm& comm);
	~GyroModule() override;

private:
	I2C& i2c;
	ModuleBus bus;
	Comm& comm;

	glm::vec3 getAccelerometer() const;
	[[maybe_unused]] int8_t getTemperature() const;

	void sleepyLoop() override;

	static constexpr uint8_t Addr = 0x18;
};


#endif //PERSE_ROVER_GYROMODULE_H
