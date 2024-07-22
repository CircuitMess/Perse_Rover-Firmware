#ifndef PERSE_ROVER_TEMPHUMMODULE_H
#define PERSE_ROVER_TEMPHUMMODULE_H

#include "Periph/I2C.h"
#include "CommData.h"
#include "Services/Comm.h"
#include <array>

class TempHumModule : private SleepyThreaded {
public:
	TempHumModule(I2C& i2c, ModuleBus bus);
	~TempHumModule() override;

private:
	I2C& i2c;
	ModuleBus bus;
	Comm* comm = nullptr;

	static constexpr uint8_t Addr = 0x38;

	std::array<uint8_t, 6> readData();

	void sleepyLoop() override;

	static float getHumidity(const std::array<uint8_t, 6>& data);

	static float getTemp(const std::array<uint8_t, 6>& data);
};


#endif //PERSE_ROVER_TEMPHUMMODULE_H
