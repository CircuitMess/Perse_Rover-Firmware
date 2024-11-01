#include "AltPressModule.h"
#include "Util/stdafx.h"
#include "Services/Modules.h"
#include "Util/Services.h"

AltPressModule::AltPressModule(I2C& i2c, ModuleBus bus) : SleepyThreaded(Modules::ModuleSendInterval, "AltPress", 2 * 1024), i2c(i2c), bus(bus),
														  comm(*((Comm*) Services.get(Service::Comm))){
	ESP_ERROR_CHECK(i2c.write(Addr, 0x06)); // soft rese)t

	delayMillis(5);

	ESP_ERROR_CHECK(i2c.write(Addr, 0b01010100));

	start();
}

AltPressModule::~AltPressModule(){
	stop();
}

int AltPressModule::readSensor(AltPressModule::Sensor sensor){
	uint8_t read[3];

	i2c.readReg(Addr, sensor, read, 3);

	const int data = (read[0] << 16) | (read[1] << 8) | read[2];

	return data / 100;
}

void AltPressModule::sleepyLoop(){
	i2c.write(Addr, 0x40);
	delayMillis(100);

	const auto alt = (int16_t) readSensor(ALTITUDE);
	const uint16_t press = readSensor(PRESSURE);

	const ModuleData data = {
			ModuleType::AltPress, bus, { .altPress = { alt, press } }
	};

	comm.sendModuleData(data);
}
