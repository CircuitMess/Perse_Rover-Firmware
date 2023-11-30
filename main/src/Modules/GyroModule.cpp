#include "GyroModule.h"
#include "Services/Modules.h"
#include "Util/Services.h"

GyroModule::GyroModule(I2C& i2c, ModuleBus bus) : SleepyThreaded(Modules::ModuleSendInterval, "Gyro", 3 * 1024), i2c(i2c), bus(bus),
												  comm(*((Comm*) Services.get(Service::Comm))){
	const uint8_t initData[2] = { 0x20, 0b01010001 };

	ESP_ERROR_CHECK(i2c.write(Addr, initData, 2));

	start();
}

GyroModule::~GyroModule(){
	stop();
}

glm::vec3 GyroModule::getAccelerometer() const{
	uint8_t data[6];

	const auto err = i2c.readReg(Addr, 0x28, data, 6);

	if(err != ESP_OK){
		return {};
	}

	const glm::vec<3, int16_t> accel = {
			(data[1] << 8) | data[0],
			(data[3] << 8) | data[2],
			(data[5] << 8) | data[4]
	};

	glm::vec3 conv = accel;
	conv *= 0.061;
	conv /= 1000;
	return conv;
}

[[maybe_unused]] int8_t GyroModule::getTemperature() const{
	int8_t temp = 0;
	ESP_ERROR_CHECK(i2c.readReg(Addr, 0x26, (uint8_t&) temp, 1));

	temp += 25;

	return temp;
}

void GyroModule::sleepyLoop(){
	auto accel = getAccelerometer();

	if(bus == ModuleBus::Left){
		accel.x *= -1;
		accel.y *= -1;
	}

	const int16_t xAngle = atan2(accel.y, accel.z) * 180 / M_PI;
	const int16_t yAngle = atan2(accel.x, accel.z) * 180 / M_PI;

	const ModuleData data = {
			ModuleType::Gyro, bus, { .gyro = { xAngle, yAngle } }
	};

	comm.sendModuleData(data);
}


