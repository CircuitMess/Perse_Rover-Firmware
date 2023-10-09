#include "AltPressModule.h"
#include "Util/stdafx.h"

AltPressModule::AltPressModule(I2C& i2c) : i2c(i2c){
	auto err = i2c.write(Addr, 0x06); // soft reset
	ESP_ERROR_CHECK(err);

	delayMillis(5);

	err = i2c.write(Addr, 0b01010100);
	ESP_ERROR_CHECK(err);
}

int AltPressModule::getPressure(){
	return readSensor(PRESSURE);
}

int AltPressModule::getAltitude(){
	return readSensor(ALTITUDE);
}

int AltPressModule::readSensor(AltPressModule::Sensor sensor){
	uint8_t read[3];

	i2c.readReg(Addr, sensor, read, 3);

	int data = (read[0] << 16) | (read[1] << 8) | read[2];

	return data / 100;
}
