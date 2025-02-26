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

void AltPressModule::sendReadSignal() const{
	i2c.write(Addr, 0b01010100);
	delayMillis(100);
}

int AltPressModule::readSensor(AltPressModule::Sensor sensor){
	uint8_t read[3] = {0};

	i2c.readReg(Addr, sensor, read, 3);

	int data = ((read[0]) << 16) | (read[1] << 8) | read[2];

	if(sensor == ALTITUDE && (data & 0x80000)){
		data |= 0xFFF00000;
	}

	return data / 100;
}

void AltPressModule::sleepyLoop(){
	sendReadSignal();

	const ModuleData data = {
			ModuleType::AltPress, bus, { .altPress = {
					(int16_t) readSensor(AltPressModule::ALTITUDE),
					(uint16_t) readSensor(AltPressModule::PRESSURE) } }
	};

	comm.sendModuleData(data);
}
