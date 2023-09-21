#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <soc/gpio_periph.h>
#include "Pins.hpp"
#include "Util/stdafx.h"
#include "Periph/WiFiAP.h"
#include "Services/TCPServer.h"
#include "Util/Services.h"
#include "Devices/Motors.h"
#include "Devices/AW9523.h"
#include "Devices/Camera.h"
#include "Periph/MCPWM.h"
#include "soc/io_mux_reg.h"
#include "Util/Events.h"
#include "Periph/WiFiAP.h"
#include "Util/Services.h"
#include "Services/TCPServer.h"
#include "Services/Comm.h"
#include "Services/PairService.h"

AW9523* aw9523;
MCPWM* motorLeft;
MCPWM* motorRight;

//input -> output, using signal from 208 to 212
void connectGPIO(gpio_num_t input, gpio_num_t output, uint32_t signal_idx){
    PIN_INPUT_ENABLE(GPIO_PIN_MUX_REG[output]);
    esp_rom_gpio_connect_in_signal(output, signal_idx, false);
    // Input pin OE to be able to connect to the signal is done by the esp_rom_gpio_connect_out_signal function
    esp_rom_gpio_connect_out_signal(input, signal_idx, false, false);
}

MCPWM::DriveMode mode = MCPWM::Brake;
void init(){

	auto ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	auto wifi = new WiFiAP();
	Services.set(Service::WiFi, wifi);
	auto tcp = new TCPServer();
	Services.set(Service::TCP, tcp);

	auto i2c = new I2C(I2C_NUM_0, (gpio_num_t) I2C_SDA, (gpio_num_t) I2C_SCL);

	aw9523 = new AW9523(*i2c, 0x5B);
	aw9523->reset();
	aw9523->write(0, false);
	aw9523->pinMode(2, AW9523::IN);


	motorLeft = new MCPWM((gpio_num_t)MOTOR_FL_A, (gpio_num_t)MOTOR_FL_B, 1);
	motorLeft->setSpeed(100);
	motorLeft->setMode(MCPWM::Forward);
	motorLeft->enable();
	connectGPIO((gpio_num_t)MOTOR_BL_A, (gpio_num_t)MOTOR_FL_A, 208);
	connectGPIO((gpio_num_t)MOTOR_BL_B, (gpio_num_t)MOTOR_FL_B, 209);

	motorRight = new MCPWM((gpio_num_t)MOTOR_FR_A, (gpio_num_t)MOTOR_FR_B, 1);
	motorRight->setSpeed(100);
	motorRight->setMode(MCPWM::Forward);
	motorRight->enable();
	connectGPIO((gpio_num_t)MOTOR_BR_A, (gpio_num_t)MOTOR_FR_A, 210);
	connectGPIO((gpio_num_t)MOTOR_BR_B, (gpio_num_t)MOTOR_FR_B, 211);



	auto comm = new Comm();
	auto pair = new PairService();
	EventQueue q(10);
	Event event{};
	Events::listen(Facility::Pair, &q);
	Events::listen(Facility::Comm, &q);


	while(1){
		if(q.get(event, portMAX_DELAY)){
			if(event.facility == Facility::Pair){
				printf("pair ok\n");
			}
			if(event.facility == Facility::Comm){
				auto data = (Comm::Event*)event.data;
				if(data->type == CommType::DriveDir){

					printf("angle: %d, value: %.2f\n", data->dir.angle, data->dir.value);

					float angle = data->dir.angle;
					if(angle > 180){
						angle -= 360;
					}
					float speed = data->dir.value;
					float leftSpeed = 0;
					float rightSpeed = 0;

					//as angle goes from 0 to -180, right motor goes from 100 to -100, from 0 to 90 is 100, from 90 to 180 is -100
					//as angle goes from 0 to 180, left motor goes from 100 to -100, from 0 to -90 is 100, from -90 to -180 is -100
					if(angle >= 0){
						leftSpeed = 100.0f * ((90 - angle) / 90.0f);
						if(angle <= 90){
							rightSpeed = 100;
						}
						if(angle >= 90){
							rightSpeed = -100;
						}
					}else{
						rightSpeed = 100.0f * ((90 - abs(angle)) / 90.0f);
						if(abs(angle) <= 90){
							leftSpeed = 100;
						}
						if(abs(angle) >= 90){
							leftSpeed = -100;
						}
					}

					leftSpeed *= speed;
					rightSpeed *= speed;
					motorLeft->setSpeed(abs(leftSpeed));
					motorRight->setSpeed(abs(rightSpeed));
					if(leftSpeed < 0){
						motorLeft->setMode(MCPWM::Reverse);
					}else{
						motorLeft->setMode(MCPWM::Forward);
					}

					if(rightSpeed < 0){
						motorRight->setMode(MCPWM::Reverse);
					}else{
						motorRight->setMode(MCPWM::Forward);
					}
				}
			}
			free(event.data);
		}
	}

}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
