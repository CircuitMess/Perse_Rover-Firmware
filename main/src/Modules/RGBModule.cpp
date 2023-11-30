#include "RGBModule.h"
#include "Util/stdafx.h"

RGBModule::RGBModule(ModuleBus bus) : SleepyThreaded(500, "RGBModule", 2 * 1024), rgb(3, bus == ModuleBus::Left ? (gpio_num_t) A_CTRL_1 : (gpio_num_t) B_CTRL_1){
	start();
	srand(millis());
}

void RGBModule::setAll(glm::vec<3, uint8_t> color){
	rgb.setAll(color);
}

void RGBModule::setPixel(glm::vec<3, uint8_t> color, uint32_t index){
	rgb.setPixel(color, index);
}

void RGBModule::push(){
	rgb.push();
}

void RGBModule::sleepyLoop(){
	constexpr float brightnessFactor = 0.2f;
	setPixel({brightnessFactor * (rand() % 0x100), brightnessFactor * (rand() % 0x100), brightnessFactor * (rand() % 0x100)}, 0);
	setPixel({brightnessFactor * (rand() % 0x100), brightnessFactor * (rand() % 0x100), brightnessFactor * (rand() % 0x100)}, 1);
	setPixel({brightnessFactor * (rand() % 0x100), brightnessFactor * (rand() % 0x100), brightnessFactor * (rand() % 0x100)}, 2);
	push();
}
