#include "JigHWTest.h"
#include "SPIFFSChecksum.hpp"
#include <Pins.hpp>
#include <soc/efuse_reg.h>
#include <esp_efuse.h>
#include <ctime>
#include <iostream>
#include <esp_mac.h>
#include "Util/Services.h"
#include <driver/adc.h>
#include <driver/gptimer.h>
#include <driver/ledc.h>
#include "Devices/Input.h"
#include "Util/Events.h"


JigHWTest* JigHWTest::test = nullptr;
I2C* JigHWTest::i2c = nullptr;
AW9523* JigHWTest::aw9523 = nullptr;
Audio* JigHWTest::audio = nullptr;


JigHWTest::JigHWTest(){
	i2c = new I2C(I2C_NUM_0, (gpio_num_t) I2C_SDA, (gpio_num_t) I2C_SCL);
	aw9523 = new AW9523(*i2c);
	audio = new Audio(*aw9523);

	test = this;

	tests.push_back({ JigHWTest::SPIFFSTest, "SPIFFS", [](){} });
	tests.push_back({ JigHWTest::AW9523Check, "AW9523", [](){} });
	tests.push_back({ JigHWTest::BatteryCalib, "Battery calibration", [](){} });
	tests.push_back({ JigHWTest::BatteryCheck, "Battery check", [](){} });
}

bool JigHWTest::checkJig(){
	char buf[7];
	int wp = 0;

	uint32_t start = millis();
	int c;
	while(millis() - start < CheckTimeout){
		vTaskDelay(1);
		c = getchar();
		if(c == EOF) continue;
		buf[wp] = (char) c;
		wp = (wp + 1) % 7;

		for(int i = 0; i < 7; i++){
			int match = 0;
			static const char* target = "JIGTEST";

			for(int j = 0; j < 7; j++){
				match += buf[(i + j) % 7] == target[j];
			}

			if(match == 7) return true;
		}
	}

	return false;
}

void JigHWTest::start(){
	uint64_t _chipmacid = 0LL;
	esp_efuse_mac_get_default((uint8_t*) (&_chipmacid));
	printf("\nTEST:begin:%llx\n", _chipmacid);

	bool pass = true;
	for(const Test& test : tests){
		currentTest = test.name;

		printf("TEST:startTest:%s\n", currentTest);

		bool result = test.test();

		printf("TEST:endTest:%s\n", result ? "pass" : "fail");

		if(!(pass &= result)){
			if(test.onFail){
				test.onFail();
			}

			break;
		}
	}

	if(!pass){
		printf("TEST:fail:%s\n", currentTest);
		vTaskDelete(nullptr);
	}

	printf("TEST:passall\n");

	//------------------------------------------------------

	AudioVisualTest();
}

void JigHWTest::log(const char* property, const char* value){
	printf("%s:%s:%s\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, float value){
	printf("%s:%s:%f\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, double value){
	printf("%s:%s:%lf\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, bool value){
	printf("%s:%s:%s\n", currentTest, property, value ? "TRUE" : "FALSE");
}

void JigHWTest::log(const char* property, uint32_t value){
	printf("%s:%s:%lu\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, int32_t value){
	printf("%s:%s:%ld\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, const std::string& value){
	printf("%s:%s:%s\n", currentTest, property, value.c_str());
}

bool JigHWTest::BatteryCalib(){
	if(Battery::getVoltOffset() != 0){
		test->log("calibrated", (int32_t) Battery::getVoltOffset());
		return true;
	}

	constexpr uint16_t numReadings = 50;
	constexpr uint16_t readDelay = 50;
	uint32_t reading = 0;

	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_6);

	for(int i = 0; i < numReadings; i++){
		reading += adc1_get_raw(ADC1_CHANNEL_1);
		vTaskDelay(readDelay / portTICK_PERIOD_MS);
	}
	reading /= numReadings;

	uint32_t mapped = Battery::mapRawReading(reading);

	int16_t offset = referenceVoltage - mapped;

	test->log("reading", reading);
	test->log("mapped", mapped);
	test->log("offset", (int32_t) offset);

	if(abs(offset) >= 1000){
		test->log("offset too big, read voltage: ", (uint32_t) mapped);
		return false;
	}

	uint8_t offsetLow = offset & 0xff;
	uint8_t offsetHigh = (offset >> 8) & 0xff;

	esp_efuse_batch_write_begin();
	esp_efuse_write_field_blob((const esp_efuse_desc_t**) efuse_adc1_low, &offsetLow, 8);
	esp_efuse_write_field_blob((const esp_efuse_desc_t**) efuse_adc1_high, &offsetHigh, 8);
	esp_efuse_batch_write_commit();

	return true;
}


bool JigHWTest::BatteryCheck(){
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_6);

	constexpr uint16_t numReadings = 50;
	constexpr uint16_t readDelay = 10;
	uint32_t reading = 0;

	for(int i = 0; i < numReadings; i++){
		reading += adc1_get_raw(ADC1_CHANNEL_1);
		vTaskDelay(readDelay / portTICK_PERIOD_MS);
	}
	reading /= numReadings;

	uint32_t voltage = Battery::mapRawReading(reading) + Battery::getVoltOffset();
	if(voltage < referenceVoltage - 100 || voltage > referenceVoltage + 100){
		test->log("raw", reading);
		test->log("mapped", (int32_t) Battery::mapRawReading(reading));
		test->log("offset", (int32_t) Battery::getVoltOffset());
		test->log("mapped+offset", voltage);
		return false;
	}

	return true;
}

bool JigHWTest::AW9523Check(){
	if(i2c->probe(0x58, 200) == ESP_OK){
		return true;
	}

	return false;
}

bool JigHWTest::SPIFFSTest(){
	auto ret = esp_vfs_spiffs_register(&spiffsConfig);
	if(ret != ESP_OK){
		test->log("spiffs", false);
		return false;
	}

	for(const auto& f : SPIFFSChecksums){
		auto file = fopen(f.name, "rb");
		if(file == nullptr){
			test->log("missing", f.name);
			return false;
		}

		uint32_t sum = calcChecksum(file);
		fclose(file);

		if(sum != f.sum){
			test->log("file", f.name);
			test->log("expected", (uint32_t) f.sum);
			test->log("got", (uint32_t) sum);

			return false;
		}
	}

	return true;
}

uint32_t JigHWTest::calcChecksum(FILE* file){
	if(file == nullptr) return 0;

#define READ_SIZE 512

	uint32_t sum = 0;
	uint8_t b[READ_SIZE];
	size_t read = 0;
	while((read = fread(b, 1, READ_SIZE, file))){
		for(int i = 0; i < read; i++){
			sum += b[i];
		}
	}

	return sum;
}

void JigHWTest::AudioVisualTest(){
	if(aw9523 == nullptr){
		return;
	}

	new Input(*aw9523);
	EventQueue queue(1);
	Events::listen(Facility::Input, &queue);
	bool mute = false;

	for(;;){
		Event evt;
		if(queue.get(evt, 0)){
			auto data = (Input::Data*) evt.data;
			if(data->action == Input::Data::Press && data->btn == Input::Button::Pair){
				mute = true;
			}
			free(evt.data);
		}

		if(!mute){
			// TODO play some audio
		}

		vTaskDelay(2000);

		if(!mute){
			// TODO stop playing audio
		}

		vTaskDelay(2000);
	}
}