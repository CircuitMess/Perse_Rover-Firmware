#include "MotionSensor.h"
#include <driver/gpio.h>
#include "esp_log.h"

MotionSensor::MotionSensor(ModuleBus bus) : pin(bus == ModuleBus::Left ? (gpio_num_t) A_CTRL_1 : (gpio_num_t) B_CTRL_1){
	gpio_config_t io_conf = {
			.pin_bit_mask = 1ULL << pin,
			.mode = GPIO_MODE_INPUT,
			.pull_up_en = GPIO_PULLUP_DISABLE,
			.pull_down_en = GPIO_PULLDOWN_ENABLE,
			.intr_type = GPIO_INTR_DISABLE
	};
	gpio_config(&io_conf);
}

bool MotionSensor::getMotion(){
	return gpio_get_level(pin);
}
