#include "MotionSensor.h"
#include <driver/gpio.h>
#include "esp_log.h"
#include "Util/Services.h"

MotionSensor::MotionSensor(ModuleBus bus) : Threaded("MotionSens", 2 * 1024),
											pin(bus == ModuleBus::Left ? (gpio_num_t) A_CTRL_1 : (gpio_num_t) B_CTRL_1), bus(bus),
											comm(*((Comm*) Services.get(Service::Comm))){
	sem = xSemaphoreCreateBinary();

	gpio_config_t io_conf = {
			.pin_bit_mask = 1ULL << pin,
			.mode = GPIO_MODE_INPUT,
			.pull_up_en = GPIO_PULLUP_DISABLE,
			.pull_down_en = GPIO_PULLDOWN_ENABLE,
			.intr_type = GPIO_INTR_ANYEDGE
	};
	gpio_intr_enable(pin);
	gpio_config(&io_conf);
	gpio_isr_handler_add(pin, isr, this);

	start();
}

MotionSensor::~MotionSensor(){
	gpio_isr_handler_remove(pin);
	gpio_intr_disable(pin);
	stop(0);
	exitFlag = true;
	xSemaphoreGive(sem);
	while(running()){
		vTaskDelay(1);
	}
	vSemaphoreDelete(sem);
}

void MotionSensor::isr(void* arg){
	MotionSensor& motion = *((MotionSensor*) arg);
	BaseType_t priority = pdFALSE;
	xSemaphoreGiveFromISR(motion.sem, &priority);
}

void MotionSensor::loop(){
	while(!xSemaphoreTake(sem, portMAX_DELAY));

	if(exitFlag) return;

	bool lvl = gpio_get_level(pin);
	ModuleData data = {
			ModuleType::Motion, bus, { .motion = { lvl } }
	};
	comm.sendModuleData(data);
}

