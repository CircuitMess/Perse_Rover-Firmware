#include "RGBModule.h"

RGBModule::RGBModule(ModuleBus bus) : rgb(3, bus == ModuleBus::Left ? (gpio_num_t) A_CTRL_1 : (gpio_num_t) B_CTRL_1){

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