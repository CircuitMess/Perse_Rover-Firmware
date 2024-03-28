#include "Modules.h"
#include "Util/Events.h"
#include <driver/gpio.h>
#include "Modules/AltPressModule.h"
#include "Modules/GyroModule.h"
#include "Modules/TempHumModule.h"
#include "Modules/LEDModule.h"
#include "Modules/RGBModule.h"
#include "Modules/PhotoresModule.h"
#include "Modules/MotionSensor.h"
#include "Modules/CO2Sensor.h"
#include "Services/TCPServer.h"
#include "Util/Services.h"
#include "Util/stdafx.h"

const std::unordered_map<ModuleType, std::pair<std::function<void*(I2C& i2c, ModuleBus bus, ADC& adc)>, std::function<void(void*)>>> ModuleConstrDestr = {
		{ ModuleType::TempHum,  { [](I2C& i2c, ModuleBus bus, ADC& adc){ return new TempHumModule(i2c, bus); },
										[](void* instance){ delete (TempHumModule*) instance; }}},
		{ ModuleType::Gyro,     { [](I2C& i2c, ModuleBus bus, ADC& adc){ return new GyroModule(i2c, bus); },
										[](void* instance){ delete (GyroModule*) instance; }}},
		{ ModuleType::AltPress, { [](I2C& i2c, ModuleBus bus, ADC& adc){ return new AltPressModule(i2c, bus); },
										[](void* instance){ delete (AltPressModule*) instance; }}},
		{ ModuleType::LED,      { [](I2C& i2c, ModuleBus bus, ADC& adc){ return new LEDModule(bus); },
										[](void* instance){ delete (LEDModule*) instance; }}},
		{ ModuleType::RGB,      { [](I2C& i2c, ModuleBus bus, ADC& adc){ return new RGBModule(bus); },
										[](void* instance){ delete (RGBModule*) instance; }}},
		{ ModuleType::PhotoRes, { [](I2C& i2c, ModuleBus bus, ADC& adc){ return new PhotoresModule(bus, adc); },
										[](void* instance){ delete (PhotoresModule*) instance; }}},
		{ ModuleType::Motion,   { [](I2C& i2c, ModuleBus bus, ADC& adc){ return new MotionSensor(bus); },
										[](void* instance){ delete (MotionSensor*) instance; }}},
		{ ModuleType::CO2,      { [](I2C& i2c, ModuleBus bus, ADC& adc){ return new CO2Sensor(bus, adc); },
										[](void* instance){ delete (CO2Sensor*) instance; }}}
};

const std::unordered_map<uint8_t, ModuleType> Modules::AddressMap = {
		{ 10, ModuleType::LED },
		{ 11, ModuleType::RGB },
		{ 12, ModuleType::PhotoRes },
		{ 13, ModuleType::Motion },
		{ 14, ModuleType::CO2 }
};

const std::unordered_map<uint8_t, ModuleType> Modules::I2CAddressMap = {
		{ 0x38, ModuleType::TempHum },
		{ 0x18, ModuleType::Gyro },
		{ 0x76, ModuleType::AltPress }
};

const std::unordered_map<ModuleType, Modules::ModuleAudio> Modules::AudioFilesMap = {
		{ ModuleType::TempHum,  { "/spiffs/Modules/TempOn.aac",      "/spiffs/Modules/TempOff.aac" }},
		{ ModuleType::Gyro,     { "/spiffs/Modules/GyroOn.aac",      "/spiffs/Modules/GyroOff.aac" }},
		{ ModuleType::AltPress, { "/spiffs/Modules/AltiOn.aac",      "/spiffs/Modules/AltiOff.aac" }},
		{ ModuleType::LED,      { "/spiffs/Modules/LedOn.aac",       "/spiffs/Modules/LedOff.aac" }},
		{ ModuleType::RGB,      { "/spiffs/Modules/RgbOn.aac",       "/spiffs/Modules/RgbOff.aac" }},
		{ ModuleType::PhotoRes, { "/spiffs/Modules/LightSensOn.aac", "/spiffs/Modules/LightSensOff.aac" }},
		{ ModuleType::Motion,   { "/spiffs/Modules/MotionOn.aac",    "/spiffs/Modules/MotionOff.aac" }},
		{ ModuleType::CO2,      { "/spiffs/Modules/AirOn.aac",       "/spiffs/Modules/AirOff.aac" }}
};

Modules::Modules(I2C& i2c, ADC& adc) : SleepyThreaded(CheckInterval, "Modules", 4 * 1024, 5, 1),
									   i2c(i2c), comm(*((Comm*) Services.get(Service::Comm))), adc(adc),
									   audio(*((Audio*) Services.get(Service::Audio))), tca(i2c),
									   connectionThread([this](){ connectionLoop(); }, "ModulesConnection", 3 * 1024, 5, 1),
									   connectionQueue(10){
	Modules::sleepyLoop();
	start();

	Events::listen(Facility::Comm, &connectionQueue);
	Events::listen(Facility::TCP, &connectionQueue);
	connectionThread.start();
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

	connectionThread.stop(0);
	connectionQueue.unblock();

	while(connectionThread.running()){
		delayMillis(1);
	}

	Events::unlisten(&connectionQueue);
}

ModuleType Modules::getInserted(ModuleBus bus){
	const auto& context = getContext(bus);

	if(!context.inserted){
		return ModuleType::Unknown;
	}

	return (ModuleType) context.current;
}

void Modules::sleepyLoop(){
	loopCheck(ModuleBus::Left);
	loopCheck(ModuleBus::Right);
}

bool Modules::checkInserted(ModuleBus bus){
	const auto scan = tca.readAll();

	const auto& context = getContext(bus);
	const bool det1 = scan & (1 << context.DetPins[0]);
	const bool det2 = scan & (1 << context.DetPins[1]);
	return det1 == 0 && det2 == 1;
}

ModuleType Modules::checkAddr(ModuleBus bus){
	const auto& context = getContext(bus);
	const auto scan = tca.readAll();

	uint8_t addr = 0;
	for(int i = 0; i < 6; i++){
		auto state = scan & (1 << context.AddrPins[i]);
		if(state){
			addr |= 1 << i;
		}
	}

	const auto& oppositeContext = getContext(bus == ModuleBus::Left ? ModuleBus::Right : ModuleBus::Left);

	if(addr != I2CModuleAddress){
		if(!AddressMap.contains(addr)){
			return ModuleType::Unknown;
		}
		return AddressMap.at(addr);
	}

	for(auto& pair: I2CAddressMap){
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
	const bool nowInserted = checkInserted(bus);
	auto& context = getContext(bus);

	if(context.inserted && !nowInserted){
		context.inserted = false;
		const auto removed = context.current;
		context.current = ModuleType::Unknown;

		if(AudioFilesMap.contains(removed)){
			audio.play(AudioFilesMap.at(removed).removedPath); //TODO - maybe set priority=true
		}

		Events::post(Facility::Modules, Event{ .action = Event::Remove, .bus = bus, .module = removed });
		if(modulesEnabled){
			comm.sendModulePlug(removed, bus, context.inserted);
		}

		if(ModuleConstrDestr.contains(removed)){
			ModuleConstrDestr.at(removed).second(context.moduleInstance);
		}
		context.moduleInstance = nullptr;

	}else if(!context.inserted && nowInserted){
		const ModuleType addr = checkAddr(bus);

		context.current = addr;
		context.inserted = true;

		if(AudioFilesMap.contains(context.current)){
			audio.play(AudioFilesMap.at(context.current).insertedPath); //TODO - maybe set priority=true
		}

		Events::post(Facility::Modules, Event{ .action = Event::Insert, .bus = bus, .module = context.current });
		if(modulesEnabled){
			comm.sendModulePlug(context.current, bus, context.inserted);
		}

		if(ModuleConstrDestr.contains(context.current)){
			context.moduleInstance = ModuleConstrDestr.at(context.current).first(i2c, bus, adc);
		}
	}
}

void Modules::connectionLoop(){
	::Event e{};
	while(!connectionQueue.get(e, portMAX_DELAY));

	if(!running()) return;

	if(e.facility == Facility::TCP){
		const auto& data = *((TCPServer::Event*) e.data);
		if(data.status != TCPServer::Event::Status::Disconnected) return;

		modulesEnabled = false;
	}else if(e.facility == Facility::Comm){
		const auto& data = *((Comm::Event*) e.data);
		if(data.type != CommType::ModulesEnable) return;
		if(modulesEnabled == (bool) data.raw) return;

		modulesEnabled = (bool) data.raw;

		if(!modulesEnabled) return;

		if(!leftContext.inserted && !rightContext.inserted) return;

		if(leftContext.inserted){
			comm.sendModulePlug(leftContext.current, ModuleBus::Left, leftContext.inserted);
		}
		if(rightContext.inserted){
			comm.sendModulePlug(rightContext.current, ModuleBus::Right, rightContext.inserted);
		}
	}

	free(e.data);
}
