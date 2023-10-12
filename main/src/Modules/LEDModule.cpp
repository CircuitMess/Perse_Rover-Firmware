#include "LEDModule.h"

LEDModule::LEDModule(ModuleBus bus) : pinout(bus == ModuleBus::Left ? A_CTRL_1 : B_CTRL_1, false){

}

void LEDModule::on(){

	pinout.on();
	printf("on\n");
}

void LEDModule::off(){
	pinout.off();
	printf("off\n");
}
