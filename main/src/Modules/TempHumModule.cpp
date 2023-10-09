#include "TempHumModule.h"
#include "Util/stdafx.h"

TempHumModule::TempHumModule(I2C& i2c) : i2c(i2c){
	auto err = i2c.write(Addr, 0x00);
	ESP_ERROR_CHECK(err);
}

float TempHumModule::getHumidity(){
	auto data = readData();

	int h = data[1] << 12 | data[2] << 4 | data[3] >> 4;
	float hum = (h * 100.0) / 0x100000;

	return hum;
}

float TempHumModule::getTemp(){
	auto data = readData();

	int t = (data[3] & 0x0F) << 16 | data[4] << 8 | data[5];
	float temp = (t * 200.0 / 0x100000) - 50;

	return temp;
}

std::array<uint8_t, 6> TempHumModule::readData(){
	std::array<uint8_t, 6> data{};
	i2c.write(Addr, { 0xAC, 0x33, 0x00 });
	delayMillis(80);
	i2c.read(Addr, &data[0], 6);

	return data;
}
