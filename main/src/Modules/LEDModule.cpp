#include "LEDModule.h"

LEDModule::LEDModule(ModuleBus bus) : pinout(bus == ModuleBus::Left ? A_CTRL_1 : B_CTRL_1, false){

}

void LEDModule::on(){
	pinout.on();
}

void LEDModule::off(){
	pinout.off();
}
