#include "JigHWTest.h"
#include "SPIFFSChecksum.hpp"
#include <Pins.hpp>
#include <soc/efuse_reg.h>
#include <esp_efuse.h>
#include <iostream>
#include <esp_mac.h>
#include "Util/Services.h"
#include <driver/adc.h>
#include <driver/ledc.h>
#include <esp_camera.h>
#include "Devices/Input.h"
#include "Util/Events.h"
#include "Util/HWVersion.h"


JigHWTest* JigHWTest::test = nullptr;
I2C* JigHWTest::i2c = nullptr;
AW9523* JigHWTest::aw9523 = nullptr;
Audio* JigHWTest::audio = nullptr;
adc_oneshot_unit_handle_t JigHWTest::hndl = nullptr;


JigHWTest::JigHWTest(){
	i2c = new I2C(I2C_NUM_0, (gpio_num_t) I2C_SDA, (gpio_num_t) I2C_SCL);
	aw9523 = new AW9523(*i2c, 0x5b);
	audio = new Audio(*aw9523);

	const gpio_config_t cfg = {
			.pin_bit_mask = 1ULL << CAM_PIN_PWDN,
			.mode = GPIO_MODE_OUTPUT
	};

	gpio_config(&cfg);
	gpio_set_level((gpio_num_t) CAM_PIN_PWDN, 1);

	test = this;

	tests.push_back({ JigHWTest::SPIFFSTest, "SPIFFS", [](){} });
	tests.push_back({ JigHWTest::CameraCheck, "Camera", [](){} });
	tests.push_back({ JigHWTest::AW9523Check, "AW9523", [](){} });
	tests.push_back({ JigHWTest::BatteryCalib, "Battery calibration", [](){} });
	tests.push_back({ JigHWTest::BatteryCheck, "Battery check", [](){} });
	tests.push_back({ JigHWTest::HWVersion, "Hardware version", [](){} });
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

	const adc_oneshot_unit_init_cfg_t config = {
			.unit_id = ADC_UNIT_1,
			.ulp_mode = ADC_ULP_MODE_DISABLE
	};

	adc_oneshot_new_unit(&config, &hndl);

	adc_unit_t unit;
	adc_channel_t chan;
	adc_oneshot_io_to_channel((gpio_num_t)BATTERY_ADC, &unit, &chan);

	adc_oneshot_chan_cfg_t cfg = {
			.atten = ADC_ATTEN_DB_11,
			.bitwidth = ADC_BITWIDTH_12
	};

	adc_oneshot_config_channel(hndl, chan, &cfg);

	for(int i = 0; i < numReadings; i++){
		int val;
		adc_oneshot_read(hndl, chan, &val);
		reading += val;
		vTaskDelay(readDelay / portTICK_PERIOD_MS);
	}
	reading /= numReadings;

	uint32_t mapped = Battery::mapRawReading(reading);

	int16_t offset = ReferenceVoltage - mapped;

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
	if(hndl == nullptr){
		const adc_oneshot_unit_init_cfg_t config = {
				.unit_id = ADC_UNIT_1,
				.ulp_mode = ADC_ULP_MODE_DISABLE
		};

		adc_oneshot_new_unit(&config, &hndl);
	}

	adc_unit_t unit;
	adc_channel_t chan;
	adc_oneshot_io_to_channel((gpio_num_t)BATTERY_ADC, &unit, &chan);

	adc_oneshot_chan_cfg_t cfg = {
			.atten = ADC_ATTEN_DB_11,
			.bitwidth = ADC_BITWIDTH_12
	};

	adc_oneshot_config_channel(hndl, chan, &cfg);

	constexpr uint16_t numReadings = 50;
	constexpr uint16_t readDelay = 10;
	uint32_t reading = 0;

	for(int i = 0; i < numReadings; i++){
		int val;
		adc_oneshot_read(hndl, chan, &val);
		reading += val;
		vTaskDelay(readDelay / portTICK_PERIOD_MS);
	}
	reading /= numReadings;

	uint32_t voltage = Battery::mapRawReading(reading) + Battery::getVoltOffset();
	if(voltage < ReferenceVoltage - 100 || voltage > ReferenceVoltage + 100){
		test->log("raw", reading);
		test->log("mapped", (int32_t) Battery::mapRawReading(reading));
		test->log("offset", (int32_t) Battery::getVoltOffset());
		test->log("mapped+offset", voltage);
		return false;
	}

	return true;
}

bool JigHWTest::AW9523Check(){
	if(i2c->probe(0x5b, 200) == ESP_OK){
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

bool JigHWTest::CameraCheck(){
	camera_config_t config;
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer = LEDC_TIMER_0;

	config.pin_pwdn = -1;
	config.pin_reset = CAM_PIN_RESET;
	config.pin_xclk = CAM_PIN_XCLK;
	config.pin_d7 = CAM_PIN_D7;
	config.pin_d6 = CAM_PIN_D6;
	config.pin_d5 = CAM_PIN_D5;
	config.pin_d4 = CAM_PIN_D4;
	config.pin_d3 = CAM_PIN_D3;
	config.pin_d2 = CAM_PIN_D2;
	config.pin_d1 = CAM_PIN_D1;
	config.pin_d0 = CAM_PIN_D0;
	config.pin_vsync = CAM_PIN_VSYNC;
	config.pin_href = CAM_PIN_HREF;
	config.pin_pclk = CAM_PIN_PCLK;

	config.xclk_freq_hz = 14000000;

	config.sccb_i2c_port = i2c->getPort();
	config.pin_sccb_sda = -1;
	config.pin_sccb_scl = -1;

	config.frame_size = FRAMESIZE_QQVGA;
	config.pixel_format = PIXFORMAT_JPEG;
	config.fb_count = 2;
	config.fb_location = CAMERA_FB_IN_PSRAM;
	config.grab_mode = CAMERA_GRAB_LATEST;
	config.jpeg_quality = 12;

	gpio_set_level((gpio_num_t) CAM_PIN_PWDN, 0);

	auto lock = i2c->lockBus();

	auto err = esp_camera_init(&config);
	if(err == ESP_ERR_NOT_FOUND){
		test->log("camera", "not found");
		return false;
	}else if(err != ESP_OK){
		test->log("camera error", esp_err_to_name(err));
		return false;
	}

	sensor_t* sensor = esp_camera_sensor_get();
	if(sensor == nullptr){
		test->log("camera", "sensor not found");
		return false;
	}

	esp_camera_deinit();
	gpio_set_level((gpio_num_t) CAM_PIN_PWDN, 1);

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

	aw9523->pinMode(EXP_LED_MOTOR_L, AW9523::LED);
	aw9523->pinMode(EXP_LED_MOTOR_R, AW9523::LED);
	aw9523->pinMode(EXP_LED_CAM, AW9523::LED);

	for(;;){
		Event evt;
		if(queue.get(evt, 0)){
			auto data = (Input::Data*) evt.data;
			if(data->action == Input::Data::Press && data->btn == Input::Button::Pair){
				mute = true;
			}
			free(evt.data);
		}

		if(!mute && audio != nullptr){
			audio->play("/spiffs/Beep3.aac", true);
		}

		aw9523->dim(EXP_LED_MOTOR_L, 100);
		aw9523->dim(EXP_LED_MOTOR_R, 100);
		aw9523->dim(EXP_LED_CAM, 100);

		vTaskDelay(1000);

		if(!mute && audio != nullptr){
			audio->stop();
		}

		aw9523->dim(EXP_LED_MOTOR_L, 0);
		aw9523->dim(EXP_LED_MOTOR_R, 0);
		aw9523->dim(EXP_LED_CAM, 0);

		vTaskDelay(1000);
	}
}

bool JigHWTest::HWVersion(){
	return HWVersion::write() && HWVersion::check();
}
