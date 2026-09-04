#ifndef CLOCKSTAR_FIRMWARE_JIGHWTEST_H
#define CLOCKSTAR_FIRMWARE_JIGHWTEST_H

#include <vector>
#include "Util/stdafx.h"
#include "Devices/Battery.h"
#include <esp_efuse.h>
#include <esp_spiffs.h>
#include "Periph/I2C.h"
#include "Devices/AW9523.h"
#include "Devices/TCA9555.h"
#include "Services/Audio.h"
#include <Pins.hpp>

struct Test {
	bool (* test)();
	const char* name;
	void (* onFail)();
};

class JigHWTest {
public:
	JigHWTest();
	static bool checkJig();
	void start();

private:
	static I2C* i2c;
	static I2C* i2cUmax;
	static AW9523* aw9523;
	static Audio* audio;
	static JigHWTest* test;
	std::vector<Test> tests;
	const char* currentTest;

	void log(const char* property, const char* value);
	void log(const char* property, float value);
	void log(const char* property, double value);
	void log(const char* property, bool value);
	void log(const char* property, uint32_t value);
	void log(const char* property, int32_t value);
	void log(const char* property, const std::string& value);

	static bool ModulesCheck();
	static bool AW9523Check();
	static bool TCA9555Check();
	static bool SPIFFSTest();
	static bool CameraCheck();
	static bool HWVersion();
	static uint32_t calcChecksum(FILE* file);

	static void AudioVisualTest();

	static constexpr uint32_t CheckTimeout = 500;

	static constexpr uint8_t AW9523Addr = 0x5b;
	static constexpr uint8_t TCA9555Addr = 0x20;

	static constexpr esp_vfs_spiffs_conf_t spiffsConfig = {
			.base_path = "/spiffs",
			.partition_label = "storage",
			.max_files = 8,
			.format_if_mount_failed = false
	};
};

#endif //CLOCKSTAR_FIRMWARE_JIGHWTEST_H
