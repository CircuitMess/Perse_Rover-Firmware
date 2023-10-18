#include "Modules.h"
#include "Util/Events.h"
#include <driver/gpio.h>
#include "Modules/AltPressModule.h"
#include "Modules/GyroModule.h"
#include "Modules/TempHumModule.h"
#include "Modules/LEDModule.h"
#include "Modules/RGBModule.h"
#include "Modules/PhotoResModule.h"
#include "Modules/MotionSensor.h"
#include "Modules/CO2Sensor.h"

const std::map<ModuleType, std::pair<std::function<void*(I2C& i2c, ModuleBus bus)>, std::function<void(void*)>>> ModuleConstrDestr = {
		{ ModuleType::TempHum,  { [](I2C& i2c, ModuleBus bus){ return new TempHumModule(i2c); },  [](void* instance){ delete (TempHumModule*) instance; } } },
		{ ModuleType::Gyro,     { [](I2C& i2c, ModuleBus bus){ return new GyroModule(i2c); },     [](void* instance){ delete (GyroModule*) instance; } } },
		{ ModuleType::AltPress, { [](I2C& i2c, ModuleBus bus){ return new AltPressModule(i2c); }, [](void* instance){ delete (AltPressModule*) instance; } } },
		{ ModuleType::LED,      { [](I2C& i2c, ModuleBus bus){ return new LEDModule(bus); },      [](void* instance){ delete (LEDModule*) instance; } } },
		{ ModuleType::RGB,      { [](I2C& i2c, ModuleBus bus){ return new RGBModule(bus); },      [](void* instance){ delete (RGBModule*) instance; } } },
		{ ModuleType::PhotoRes, { [](I2C& i2c, ModuleBus bus){ return new PhotoresModule(bus); }, [](void* instance){ delete (PhotoresModule*) instance; } } },
		{ ModuleType::Motion,   { [](I2C& i2c, ModuleBus bus){ return new MotionSensor(bus); },   [](void* instance){ delete (MotionSensor*) instance; } } },
		{ ModuleType::CO2,      { [](I2C& i2c, ModuleBus bus){ return new CO2Sensor(bus); },      [](void* instance){ delete (CO2Sensor*) instance; } } }
};

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

Modules::Modules(ShiftReg& shiftReg, I2C& i2c, Comm& comm) : SleepyThreaded(CheckInterval, "Modules", 2 * 1024, 5, 1), shiftReg(shiftReg), i2c(i2c), comm(comm){
	Modules::sleepyLoop();
	start();
}

Modules::~Modules(){
	stop();
	if(ModuleConstrDestr.contains(leftContext.current)){
		ModuleConstrDestr.at(leftContext.current).second(leftContext.moduleInstance);
		leftContext.moduleInstance = nullptr;
	}
	if(ModuleConstrDestr.contains(rightContext.current)){
		ModuleConstrDestr.at(rightContext.current).second(rightContext.moduleInstance);
		rightContext.moduleInstance = nullptr;
	}
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
		return leftContext;
	}else{
		return rightContext;
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
		comm.sendModulePlug(removed, bus, context.inserted);

		if(ModuleConstrDestr.contains(removed)){
			ModuleConstrDestr.at(removed).second(context.moduleInstance);
		}
		context.moduleInstance = nullptr;

	}else if(!context.inserted && nowInserted){
		ModuleType addr = checkAddr(bus);

		context.current = addr;
		context.inserted = true;

		Events::post(Facility::Modules, Event{ .action = Event::Insert, .bus = bus, .module = context.current });
		comm.sendModulePlug(context.current, bus, context.inserted);

		if(ModuleConstrDestr.contains(context.current)){
			ModuleConstrDestr.at(context.current).first(i2c, bus);
		}
	}
}
