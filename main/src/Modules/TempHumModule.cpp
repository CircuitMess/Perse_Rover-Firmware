#include "TempHumModule.h"
#include "Util/stdafx.h"
#include "Services/Modules.h"
#include "Util/Services.h"

TempHumModule::TempHumModule(I2C& i2c, ModuleBus bus) : SleepyThreaded(Modules::ModuleSendInterval, "TempHum", 2 * 1024),
														i2c(i2c), bus(bus), comm(*((Comm*) Services.get(Service::Comm))){
	ESP_ERROR_CHECK(i2c.write(Addr, 0x00));

	start();
}

TempHumModule::~TempHumModule(){
	stop();
}

void TempHumModule::sleepyLoop(){
	const auto data = readData();
	const int16_t temp = (int16_t) getTemp(data);
	const uint16_t humidity = (uint16_t) getHumidity(data);

	const ModuleData md = {
			ModuleType::TempHum, bus, { .tempHum = { temp, humidity } }
	};

	comm.sendModuleData(md);
}

float TempHumModule::getHumidity(const std::array<uint8_t, 6>& data){
	const int h = data[1] << 12 | data[2] << 4 | data[3] >> 4;
	const float hum = (h * 100.0) / 0x100000;

	return hum;
}

float TempHumModule::getTemp(const std::array<uint8_t, 6>& data){
	const int t = (data[3] & 0x0F) << 16 | data[4] << 8 | data[5];
	const float temp = (t * 200.0 / 0x100000) - 50;

	return temp;
}

std::array<uint8_t, 6> TempHumModule::readData(){
	std::array<uint8_t, 6> data{};
	i2c.write(Addr, { 0xAC, 0x33, 0x00 });
	delayMillis(80);
	i2c.read(Addr, &data[0], 6);

	return data;
}
