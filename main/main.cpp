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
#include "Services/TCPServer.h"
#include "Services/Audio.h"
#include "Devices/HeadlightsController.h"
#include "Devices/Motors.h"
#include "Devices/MotorDriveController.h"
#include "Services/Comm.h"

void init(){
	gpio_config_t cfg = {
			.pin_bit_mask = MOTOR_BL_A | MOTOR_BR_A | MOTOR_BL_B | MOTOR_BR_B | MOTOR_FL_A | MOTOR_FL_B | MOTOR_FR_A | MOTOR_FR_B,
			.mode = GPIO_MODE_OUTPUT
	};
	gpio_config(&cfg);

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

	auto audio = new Audio(*aw9523);
	Services.set(Service::Audio, audio);

	auto input = new Input(*aw9523);

	auto comm = new Comm();
	Services.set(Service::Comm, comm);

	auto headlightsController = new HeadlightsController(aw9523);

	auto motorDriveController = new MotorDriveController();
}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
