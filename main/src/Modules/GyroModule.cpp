#include "GyroModule.h"

GyroModule::GyroModule(I2C& i2c) : i2c(i2c){
	const uint8_t initData[2] = { 0x20, 0b01010001 };

	auto err = i2c.write(Addr, initData, 2);

	ESP_ERROR_CHECK(err);
}

glm::vec3 GyroModule::getAccelerometer() const{
	uint8_t data[6];

	auto err = i2c.readReg(Addr, 0x28, data, 6);

	if(err != ESP_OK){
		return {};
	}

	glm::vec<3, int16_t> accel = {
			(data[1] << 8) | data[0],
			(data[3] << 8) | data[2],
			(data[5] << 8) | data[4]
	};

	glm::vec3 conv = accel;
	conv *= 0.061;
	conv /= 1000;
	return conv;
}

int8_t GyroModule::getTemperature() const{
	int8_t temp = 0;
	auto err = i2c.readReg(Addr, 0x26, (uint8_t&)temp, 1);

	ESP_ERROR_CHECK(err);

	temp += 25;

	return temp;
}


