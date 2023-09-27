#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <soc/gpio_periph.h>
#include <cmath>
#include "Pins.hpp"
#include "Util/stdafx.h"
#include "Periph/WiFiAP.h"
#include "Services/TCPServer.h"
#include "Util/Services.h"
#include "Devices/Motors.h"
#include "Devices/AW9523.h"
#include "Devices/Camera.h"
#include "Devices/Servo.h"
#include "Periph/MCPWM.h"
#include "soc/io_mux_reg.h"
#include "Util/Events.h"
#include "Periph/WiFiAP.h"
#include "Util/Services.h"
#include "Services/TCPServer.h"
#include "Services/Comm.h"
#include "Services/PairService.h"
#include "Services/Feed.h"

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

	auto leds = { 10, 11, 6, 7, 12, 13, 14, 15 };
	for(auto led : leds){
		aw9523->pinMode(led, AW9523::LED);
		aw9523->dim(led, 0);
	}

	delayMillis(800);

	for(int i = 0; i < 2; i++){
		for(auto led : leds) aw9523->dim(led, 150);
		delayMillis(150);
		for(auto led : leds) aw9523->dim(led, 0);
		delayMillis(300);
	}

	for(auto pin : { EXP_HEADLIGHT_1, EXP_HEADLIGHT_2 }){
		aw9523->pinMode(pin, AW9523::LED);
		aw9523->dim(pin, 0);
	}

	for(auto pin : { 15, 16, 17, 18, 21, 37, 47, 33 }){
		gpio_config_t cfg = {
				.pin_bit_mask = 1ULL << pin,
				.mode = GPIO_MODE_INPUT,
		};
		gpio_config(&cfg);
	}


	motorLeft = new MCPWM((gpio_num_t)MOTOR_FL_B, (gpio_num_t)MOTOR_FL_A, 1);
	motorLeft->setSpeed(0);
	motorLeft->setMode(MCPWM::Brake);
	motorLeft->enable();
	connectGPIO((gpio_num_t)MOTOR_BL_A, (gpio_num_t)MOTOR_FL_A, 208);
	connectGPIO((gpio_num_t)MOTOR_BL_B, (gpio_num_t)MOTOR_FL_B, 209);

	motorRight = new MCPWM((gpio_num_t)MOTOR_FR_A, (gpio_num_t)MOTOR_FR_B, 1);
	motorRight->setSpeed(0);
	motorRight->setMode(MCPWM::Brake);
	motorRight->enable();
	connectGPIO((gpio_num_t)MOTOR_BR_A, (gpio_num_t)MOTOR_FR_A, 210);
	connectGPIO((gpio_num_t)MOTOR_BR_B, (gpio_num_t)MOTOR_FR_B, 211);

	auto arm = new Servo((gpio_num_t) SERVO_1_PWM, 0);
	auto pinch = new Servo((gpio_num_t) SERVO_2_PWM, 0);

	arm->enable();
	arm->setValue(50);

	pinch->enable();
	pinch->setValue(50);

	auto comm = new Comm();
	auto pair = new PairService();
	auto q = new EventQueue(10);
	Events::listen(Facility::Pair, q);
	Events::listen(Facility::Comm, q);

	auto cam = new Camera(*i2c, *aw9523);
	cam->setRes(FRAMESIZE_QQVGA);
	cam->setFormat(PIXFORMAT_JPEG);
	printf("Cam init: %d\n", cam->init());

	auto feed = new Feed();
	auto feedSender = new ThreadedClosure([feed, cam](){
		auto frame = cam->getFrame();
		if(frame){
			DriveInfo info;
			info.mode = DriveMode::Manual;
			info.frame.size = frame->len;
			info.frame.data = malloc(frame->len);
			memcpy(info.frame.data, frame->buf, frame->len);
			cam->releaseFrame();

			feed->sendFrame(info);
		}
		delayMillis(20);
	}, "FeedSender", 8000, 5, 1);
	feedSender->start();

	auto evtReceiver = new ThreadedClosure([q, arm, pinch](){
		Event event{};
		if(q->get(event, portMAX_DELAY)){
			if(event.facility == Facility::Pair){
				printf("pair ok\n");
			}
			if(event.facility == Facility::Comm){
				auto data = (Comm::Event*)event.data;
				if(data->type == CommType::DriveDir){

					// printf("angle: %d, value: %.2f\n", data->dir.angle, data->dir.value);

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
						leftSpeed = 100.0f * ((90.0f - angle) / 90.0f);
						rightSpeed = 100;
					}else{
						rightSpeed = 100.0f * ((90.0f - abs(angle)) / 90.0f);
						leftSpeed = 100;
					}

					//printf("Angle %d, left: %d, right: %d\n", (int) angle, (int) leftSpeed, (int) rightSpeed);

					leftSpeed *= speed;
					rightSpeed *= speed;
					leftSpeed = std::clamp(leftSpeed, -100.0f, 100.0f);
					rightSpeed = std::clamp(rightSpeed, -100.0f, 100.0f);

					motorLeft->setSpeed(abs(rightSpeed));
					motorRight->setSpeed(abs(leftSpeed));

					if(rightSpeed < 0){
						motorLeft->setMode(MCPWM::Reverse);
					}else{
						motorLeft->setMode(MCPWM::Forward);
					}
					if(leftSpeed < 0){
						motorRight->setMode(MCPWM::Reverse);
					}else{
						motorRight->setMode(MCPWM::Forward);
					}
				}else if(data->type == CommType::Headlights){
					for(auto pin : { EXP_HEADLIGHT_1, EXP_HEADLIGHT_2 }){
						aw9523->dim(pin, data->headlights);
					}
				}else if(data->type == CommType::Arm){
					auto mapped = (int) map((float) (100 - data->arm), 0, 100, 0, 80);
					printf("Arm %d - %d\n", data->arm, mapped);
					arm->setValue(mapped);
				}else if(data->type == CommType::Pinch){
					auto mapped = (int) map((float) data->pinch, 0, 100, 32, 55);
					printf("Pinch %d - %d\n", data->pinch, mapped);
					pinch->setValue(mapped);
				}
			}
			free(event.data);
		}
	}, "FeedSender", 8000, 5, 0);
	evtReceiver->start();

}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
