#include "Modules.h"
#include "Util/Events.h"
#include <driver/gpio.h>

const std::map<uint8_t, ModuleType> Modules::AddressMap = {
		{ 10, ModuleType::LED },
		{ 11, ModuleType::RGB },
		{ 12, ModuleType::PhotoRes },
		{ 13, ModuleType::Motion },
		{ 14, ModuleType::CO2 }
};

const std::map<uint8_t, ModuleType> Modules::I2CAddressMap = {
		{ 0x38, ModuleType::TempHum },
		{ 0x18, ModuleType::Gyro },
		{ 0x76, ModuleType::AltPress }
};

Modules::Modules(ShiftReg& shiftReg, I2C& i2c) : SleepyThreaded(CheckInterval, "Modules", 2 * 1024, 5, 1), shiftReg(shiftReg), i2c(i2c){
	Modules::sleepyLoop();
	start();
}

Modules::~Modules(){
	stop();
}

ModuleType Modules::getInserted(ModuleBus bus){
	auto& context = getContext(bus);
	if(!context.inserted) return ModuleType::Unknown;
	return (ModuleType) context.current;
}

void Modules::sleepyLoop(){
	loopCheck(ModuleBus::Left);
	loopCheck(ModuleBus::Right);
}

bool Modules::checkInserted(ModuleBus bus){
	shiftReg.scan();

	auto& context = getContext(bus);
	bool det1 = shiftReg.get(context.DetPins[0]);
	bool det2 = shiftReg.get(context.DetPins[1]);
	return det1 == 0 && det2 == 1;
}

ModuleType Modules::checkAddr(ModuleBus bus){
	auto& context = getContext(bus);
	uint8_t addr = 0;
	for(int i = 0; i < 6; i++){
		auto state = shiftReg.get(context.AddrPins[i]);
		if(state){
			addr |= 1 << i;
		}
	}

	printf("addr: %d\n", addr);

	auto& oppositeContext = getContext(bus == ModuleBus::Left ? ModuleBus::Right : ModuleBus::Left);

	if(addr != I2CModuleAddress){
		if(!AddressMap.contains(addr)){
			return ModuleType::Unknown;
		}
		return AddressMap.at(addr);
	}

	for(auto& pair : I2CAddressMap){
		if(oppositeContext.current == pair.second) continue;

		if(i2c.probe(pair.first) == ESP_OK){
			return pair.second;
		}
	}
	return ModuleType::Unknown;
}

Modules::BusContext& Modules::getContext(ModuleBus bus){
	if(bus == ModuleBus::Left){
		return LeftContext;
	}else{
		return RightContext;
	}
}

void Modules::loopCheck(ModuleBus bus){
	bool nowInserted = checkInserted(bus);
	auto& context = getContext(bus);


	if(context.inserted && !nowInserted){
		context.inserted = false;
		auto removed = context.current;
		context.current = ModuleType::Unknown;

		Events::post(Facility::Modules, Event{ .action = Event::Remove, .bus = bus, .module = removed });
	}else if(!context.inserted && nowInserted){
		ModuleType addr = checkAddr(bus);

		context.current = addr;
		context.inserted = true;

		Events::post(Facility::Modules, Event{ .action = Event::Insert, .bus = bus, .module = context.current });
	}
}
