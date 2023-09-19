#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include "Pins.hpp"
#include "Util/stdafx.h"
#include "Periph/WiFiAP.h"
#include "Services/TCPServer.h"
#include "Util/Services.h"
#include "Services/PairService.h"
#include "Util/Events.h"
#include "Services/Comm.h"


void init(){
	gpio_config_t cfg = {
			.pin_bit_mask = 0,
			.mode = GPIO_MODE_INPUT
	};
	gpio_config(&cfg);
	esp_log_level_set("TCPServer", ESP_LOG_VERBOSE);
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
				auto data = (ControlPacket*)event.data;
				printf("packet type: %d, data: %d\n", (int)data->type, data->data);
			}
			free(event.data);
		}
	}


}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
