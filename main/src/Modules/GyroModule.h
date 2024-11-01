#ifndef PERSE_ROVER_GYROMODULE_H
#define PERSE_ROVER_GYROMODULE_H

#include <glm.hpp>
#include "Periph/I2C.h"
#include "Services/Comm.h"
#include "Services/Audio.h"

class GyroModule : private SleepyThreaded {
public:
	GyroModule(I2C& i2c, ModuleBus bus);
	~GyroModule() override;

private:
	I2C& i2c;
	ModuleBus bus;
	Comm* comm = nullptr;
	Audio* audio = nullptr;

	glm::vec3 value;
	float emaA = 0.75f;

	size_t loopCounter = 0;

	glm::vec3 getAccelerometer() const;
	[[maybe_unused]] int8_t getTemperature() const;

	void sleepyLoop() override;

	static constexpr uint8_t Addr = 0x18;

	bool tilted = false;
	static constexpr uint8_t TiltThreshold = 20;
};


#endif //PERSE_ROVER_GYROMODULE_H
