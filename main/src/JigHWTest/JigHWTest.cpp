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
#include "Services/Modules.h"
#include "Devices/Power.h"


JigHWTest* JigHWTest::test = nullptr;
I2C* JigHWTest::i2c = nullptr;
I2C* JigHWTest::i2cUmax = nullptr;
AW9523* JigHWTest::aw9523 = nullptr;
Audio* JigHWTest::audio = nullptr;


JigHWTest::JigHWTest(){
	// The board cuts its own power as soon as the button is released unless the latch is held.
	Power::hold();

	i2c = new I2C(I2C_NUM_0, (gpio_num_t) I2C_SDA, (gpio_num_t) I2C_SCL);
	i2cUmax = new I2C(I2C_NUM_1, (gpio_num_t) I2C_UMAX_SDA, (gpio_num_t) I2C_UMAX_SCL);
	aw9523 = new AW9523(*i2c, AW9523Addr);
	audio = new Audio(*aw9523);

	const gpio_config_t cfg = {
			.pin_bit_mask = 1ULL << CAM_PIN_PWDN,
			.mode = GPIO_MODE_OUTPUT
	};

	gpio_config(&cfg);
	gpio_set_level((gpio_num_t) CAM_PIN_PWDN, 1);

	test = this;

	tests.push_back({ JigHWTest::SPIFFSTest, "SPIFFS", [](){} });
//	tests.push_back({ JigHWTest::CameraCheck, "Camera", [](){} });
	tests.push_back({ JigHWTest::AW9523Check, "AW9523", [](){} });
	tests.push_back({ JigHWTest::TCA9555Check, "XL9555", [](){} });
//	tests.push_back({JigHWTest::ModulesCheck, "Modules", [](){}});
	/* There is no battery calibration or check on this board: the battery divider ends on a digital
	 * expander pin instead of an ADC, so there is nothing to calibrate. See Devices/Battery.h. */
	tests.push_back({ JigHWTest::HWVersion, "Hardware version", [](){ esp_efuse_batch_write_cancel(); } });
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

	esp_efuse_batch_write_begin();

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

	esp_efuse_batch_write_commit();
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

bool JigHWTest::ModulesCheck(){
	ADC adc(ADC_UNIT_1);
	TCA9555 tca(*i2c, TCA9555Addr);
	Modules modules(tca, *i2cUmax, adc);

	aw9523->pinMode(EXP_LED_MOTOR_L, AW9523::LED);
	aw9523->dim(EXP_LED_MOTOR_L, 0);

	if(modules.getInserted(Modules::Bus) != ModuleType::TempHum){
		test->log("module", "not temperature/humidity");
		return false;
	}

	aw9523->dim(EXP_LED_MOTOR_L, 100);

	EventQueue evts(12);
	Events::listen(Facility::Modules, &evts);

	const auto out = [&evts](){
		Events::unlisten(&evts);
		aw9523->dim(EXP_LED_MOTOR_L, 0);
		vTaskDelay(500);
	};

	auto waitEvt = [&evts](){
		Modules::Event mEvt{};
		for(;;){
			Event evt{};
			if(!evts.get(evt, portMAX_DELAY)) continue;
			mEvt = *((Modules::Event*) evt.data);
			free(evt.data);
			break;
		}
		return mEvt;
	};

	enum ModuleTestState {
		Start, Removed, Reinserted
	};
	ModuleTestState state = Start;

	while(state != Reinserted){
		auto evt = waitEvt();

		if(evt.module != ModuleType::TempHum){
			test->log("module", "not temperature/humidity");
			out();
			return false;
		}

		if(evt.action == Modules::Event::Remove){
			if(state != Start){
				test->log("module", "double removal!");
				out();
				return false;
			}

			state = Removed;
			aw9523->dim(EXP_LED_MOTOR_L, 0);
		}else if(evt.action == Modules::Event::Insert){
			if(state != Removed){
				test->log("module", "double insert!");
				out();
				return false;
			}

			state = Reinserted;
			aw9523->dim(EXP_LED_MOTOR_L, 100);
		}
	}

	Events::unlisten(&evts);
	aw9523->dim(EXP_LED_MOTOR_L, 0);
	return true;
}

bool JigHWTest::AW9523Check(){
	if(i2c->probe(AW9523Addr, 200) == ESP_OK){
		return true;
	}

	return false;
}

bool JigHWTest::TCA9555Check(){
	if(i2c->probe(TCA9555Addr, 200) == ESP_OK){
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

	new Input();
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
			if(data->action == Input::Data::Press && data->btn == Input::Button::Power){
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
	uint16_t version = 1;
	bool result = HWVersion::readVersion(version);

	if(!result){
		test->log("HW version", "couldn't read from efuse");
		return false;
	}

	if(version != 0){
		test->log("Existing HW version", (uint32_t) version);
		if(version == HWVersion::getHardcodedVersion()){
			test->log("Already fused.", (uint32_t) version);
			return true;
		}else{
			test->log("Wrong binary already fused!", (uint32_t) version);
			return false;
		}
	}

	return HWVersion::write();
}
