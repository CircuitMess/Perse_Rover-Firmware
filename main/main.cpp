#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_sleep.h>
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
#include "Devices/TCA9555.h"
#include "Services/TCPServer.h"
#include "Services/Comm.h"
#include "Services/Audio.h"
#include "States/PairState.h"
#include "Services/StateMachine.h"
#include "Services/LEDService.h"
#include "Services/Feed.h"
#include "Services/Modules.h"

[[noreturn]] void shutdown(){
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RC_FAST, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_CPU, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL));
	esp_deep_sleep_start();
}

void init(){
	auto adc1 = new ADC(ADC_UNIT_1);

	auto battery = new Battery(*adc1);
	if(battery->isShutdown()){
		shutdown();
		return;
	}

	Services.set(Service::Battery, battery);

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

	auto i2c = new I2C(I2C_NUM_0, (gpio_num_t) I2C_SDA, (gpio_num_t) I2C_SCL);
	auto aw9523 = new AW9523(*i2c, 0x5b);

	auto feed = new Feed(*i2c);
	Services.set(Service::Feed, feed);

	auto audio = new Audio(*aw9523);
	Services.set(Service::Audio, audio);

	auto led = new LEDService(*aw9523);
	Services.set(Service::LED, led);

	led->breathe(LED::Rear);

	auto input = new Input(*aw9523);
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

	auto modules = new Modules(*i2c, *adc1);
	Services.set(Service::Modules, modules);

	auto stateMachine = new StateMachine();
	Services.set(Service::StateMachine, stateMachine);

	stateMachine->transition<PairState>();
	stateMachine->begin();

	battery->begin();
}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
