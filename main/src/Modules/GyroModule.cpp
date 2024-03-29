#include "GyroModule.h"
#include "Services/Modules.h"
#include "Util/Services.h"

GyroModule::GyroModule(I2C& i2c, ModuleBus bus) : SleepyThreaded(50, "Gyro", 3 * 1024), i2c(i2c), bus(bus),
												  comm(*((Comm*) Services.get(Service::Comm))),
												  audio(*((Audio*) Services.get(Service::Audio))){
	const uint8_t initData[2] = { 0x20, 0b01010001 };

	ESP_ERROR_CHECK(i2c.write(Addr, initData, 2));

	value = getAccelerometer();

	if(bus == ModuleBus::Left){
		value.x *= -1;
		value.y *= -1;
	}

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

	value = value * (1.0f - emaA) + emaA * accel;

	++loopCounter;

	if(loopCounter == 10){
		const int16_t xAngle = atan2(value.y, value.z) * 180 / M_PI;
		const int16_t yAngle = atan2(value.x, value.z) * 180 / M_PI;

		const ModuleData data = {
				ModuleType::Gyro, bus, { .gyro = { xAngle, yAngle }}
		};

		if((abs(xAngle) >= TiltThreshold || abs(yAngle) >= TiltThreshold) && !tilted){
			tilted = true;
			audio.play("/spiffs/Modules/GyroTilt.aac");
		}else if(abs(xAngle) < (TiltThreshold - 5) && abs(yAngle) < (TiltThreshold - 5) && tilted){
			tilted = false;
		}

		comm.sendModuleData(data);

		loopCounter = 0;
	}
}


