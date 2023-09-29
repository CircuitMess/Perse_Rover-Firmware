#include "Modules.h"
#include "Util/Events.h"
#include <driver/gpio.h>

Modules::Modules(ShiftReg& shiftReg) : SleepyThreaded(CheckInterval, "Modules", 2 * 1024, 5, 1), shiftReg(shiftReg){
	Modules::sleepyLoop();
	start();
}

Modules::~Modules(){
	stop();
}

Module Modules::getInserted(ModuleBus bus){
	auto& context = getContext(bus);
	if(!context.inserted) return Module::COUNT;
	return (Module) context.current;
}

void Modules::sleepyLoop(){
	loopCheck(ModuleBus::Bus_A);
	loopCheck(ModuleBus::Bus_B);
}

bool Modules::checkInserted(ModuleBus bus){
	shiftReg.scan();

	auto& context = getContext(bus);
	bool det1 = shiftReg.get(context.DetPins[0]);
	bool det2 = shiftReg.get(context.DetPins[1]);
	return det1 == 0 && det2 == 1;
}

uint8_t Modules::checkAddr(ModuleBus bus){
	auto& context = getContext(bus);
	uint8_t addr = 0;
	for(int i = 0; i < 6; i++){
		auto state = gpio_get_level((gpio_num_t) context.AddrPins[i]);
		if(state){
			addr |= 1 << i;
		}
	}
	return addr;
}

Modules::BusContext& Modules::getContext(ModuleBus bus){
	if(bus == ModuleBus::Bus_A){
		return A_context;
	}else{
		return B_context;
	}
}

void Modules::loopCheck(ModuleBus bus){
	bool nowInserted = checkInserted(bus);
	auto& context = getContext(bus);


	if(context.inserted && !nowInserted){
		context.inserted = false;
		context.current = -1;

		Events::post(Facility::Modules, Event{ .action = Event::Remove, .bus = bus });
	}else if(!context.inserted && nowInserted){
		uint8_t addr = checkAddr(bus);
		if(addr >= (uint8_t) Module::COUNT){
			// Unknown module
			addr = (uint8_t) Module::COUNT;
		}

		context.current = addr;
		context.inserted = true;

		Events::post(Facility::Modules, Event{ .action = Event::Insert, .bus = bus, .module = Module(context.current) });
	}
}
