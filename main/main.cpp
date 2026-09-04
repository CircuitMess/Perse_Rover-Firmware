#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include "Util/Services.h"
#include "Util/stdafx.h"
#include "Pins.hpp"
#include "Periph/WiFiAP.h"
#include "Periph/I2C.h"
#include "Periph/SPIFFS.h"
#include "Devices/Input.h"
#include "Devices/AW9523.h"
#include "Devices/Motors.h"
#include "Devices/MotorDriveController.h"
#include "Devices/HeadlightsController.h"
#include "Devices/ArmController.h"
#include "Devices/CameraController.h"
#include "Devices/Battery.h"
#include "Devices/Power.h"
#include "Devices/TCA9555.h"
#include "Services/TCPServer.h"
#include "Services/Comm.h"
#include "Services/Audio.h"
#include "States/PairState.h"
#include "Services/StateMachine.h"
#include "Services/LEDService.h"
#include "Services/Feed.h"
#include "Services/Modules.h"
#include "States/DemoState.h"
#include "Services/BatteryLowService.h"
#include "Services/PowerButtonService.h"
#include "JigHWTest/JigHWTest.h"
#include "Services/InactivityService.h"
#include "Util/HWVersion.h"
#include "Settings.h"

[[noreturn]] void shutdown(){
	Power::off();
}

/**
 * Brings the rover down in an orderly fashion: stop driving, kill the link, turn the lights off,
 * then release the power latch. Shared by the low battery watchdog, the inactivity timeout and the
 * power button.
 */
[[noreturn]] void gracefulShutdown(const char* audioFile){
	if(TCPServer* tcp = (TCPServer*) Services.get(Service::TCP)){
		tcp->disconnect();
	}

	if(StateMachine* stateMachine = (StateMachine*) Services.get(Service::StateMachine)){
		Services.set(Service::StateMachine, nullptr);
		delete stateMachine;
	}

	if(MotorDriveController* motors = (MotorDriveController*) Services.get(Service::MotorDriveController)){
		motors->setControl(Local);
		motors->setLocally({});
	}

	if(BatteryLowService* lowBatteryService = (BatteryLowService*) Services.get(Service::LowBattery)){
		Services.set(Service::LowBattery, nullptr);
		delete lowBatteryService;
	}

	if(LEDService* led = (LEDService*) Services.get(Service::LED)){
		for(int i = 0; i < (uint8_t) LED::COUNT; i++){
			led->off((LED) i);
		}
	}

	if(Audio* audio = (Audio*) Services.get(Service::Audio)){
		Services.set(Service::Audio, nullptr);
		if(audioFile != nullptr){
			audio->play(audioFile, true);
			delayMillis(3000);
		}
		delete audio;
	}

	shutdown();
}

void init(){

	esp_log_level_set("*", ESP_LOG_WARN);
	esp_log_level_set("camera", ESP_LOG_INFO);
	esp_log_level_set("gc2145", ESP_LOG_INFO);
	esp_log_level_set("Camera", ESP_LOG_INFO);



	/* The board only stays powered while the latch is driven, so this has to happen before anything
	 * that can take time. The bootloader hook already asserted it - this claims the pin properly. */
	Power::hold();

	if(JigHWTest::checkJig()){
		printf("Jig\n");
		auto test = new JigHWTest();
		test->start();
		vTaskDelete(nullptr);
	}

	if(!HWVersion::check()){
		while(true){
			vTaskDelay(1000);
			HWVersion::log();
		}
	}


	auto settings = new Settings();
	Services.set(Service::Settings, settings);

	auto adc1 = new ADC(ADC_UNIT_1);

	auto i2c = new I2C(I2C_NUM_0, (gpio_num_t) I2C_SDA, (gpio_num_t) I2C_SCL);
	auto i2cUmax = new I2C(I2C_NUM_1, (gpio_num_t) I2C_UMAX_SDA, (gpio_num_t) I2C_UMAX_SCL);
	auto aw9523 = new AW9523(*i2c, 0x5b);
	auto tca = new TCA9555(*i2c);

	auto battery = new Battery(*tca);
	Services.set(Service::Battery, battery);

	auto led = new LEDService(*aw9523);
	Services.set(Service::LED, led);

	led->on(LED::Arm);

	// Purely decorative, on for as long as the rover is.
	led->on(LED::Deco1);
	led->on(LED::Deco2);
	led->on(LED::Deco3);

	gpio_install_isr_service(ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_SHARED | ESP_INTR_FLAG_IRAM);

	auto ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	auto spiffs = new SPIFFS();

	auto wifi = new WiFiAP();
	Services.set(Service::WiFi, wifi);
	auto tcp = new TCPServer();
	Services.set(Service::TCP, tcp);

	auto feed = new Feed(*i2c);
	Services.set(Service::Feed, feed);

	auto audio = new Audio(*aw9523);
	Services.set(Service::Audio, audio);

	led->breathe(LED::Rear);

	auto input = new Input();
	Services.set(Service::Input, input);

	auto comm = new Comm();
	Services.set(Service::Comm, comm);

	auto headlightsController = new HeadlightsController();
	Services.set(Service::HeadLightsController, headlightsController);

	auto motorDriveController = new MotorDriveController();
	Services.set(Service::MotorDriveController, motorDriveController);

	auto armController = new ArmController();
	Services.set(Service::ArmController, armController);

	auto cameraController = new CameraController();
	Services.set(Service::CameraController, cameraController);

	auto modules = new Modules(*tca, *i2cUmax, *adc1);
	Services.set(Service::Modules, modules);

	auto stateMachine = new StateMachine();
	Services.set(Service::StateMachine, stateMachine);

	auto lowBatteryService = new BatteryLowService();
	Services.set(Service::LowBattery, lowBatteryService);

	auto inactivityService = new InactivityService();

	auto powerButtonService = new PowerButtonService();

	audio->play("/spiffs/General/PowerOn.aac", true);

	stateMachine->transition<PairState>();
	stateMachine->begin();

	battery->setShutdownCallback([](){
		gracefulShutdown("/spiffs/General/BattEmptyRover.aac");
	});

	battery->begin();
}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
