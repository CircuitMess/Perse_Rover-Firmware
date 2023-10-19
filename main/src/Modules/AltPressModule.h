#ifndef PERSE_ROVER_ALTPRESS_H
#define PERSE_ROVER_ALTPRESS_H

#include "Periph/I2C.h"
#include "Util/Threaded.h"
#include "Services/Comm.h"

class AltPressModule : private SleepyThreaded {
public:
	AltPressModule(I2C& i2c, ModuleBus bus, Comm& comm);
	~AltPressModule() override;

private:
	I2C& i2c;
	const ModuleBus bus;
	Comm& comm;

	void sleepyLoop() override;
	enum Sensor {
		PRESSURE = 0x30, ALTITUDE = 0x31
	};
	int readSensor(Sensor sensor);

	static constexpr uint8_t Addr = 0x76;
};


#endif //PERSE_ROVER_ALTPRESS_H
