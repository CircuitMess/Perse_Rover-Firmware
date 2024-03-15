#include "BatteryLowService.h"
#include "LEDService.h"
#include "Util/Services.h"
#include "Devices/Battery.h"
#include "Util/stdafx.h"

BatteryLowService::BatteryLowService() : Threaded("BattLowService", 2 * 1024), queue(6){
	Events::listen(Facility::Battery, &queue);

	if(const Battery* battery = (Battery*) Services.get(Service::Battery)){
		const Battery::Level level = battery->getLevel();

		if(level == Battery::VeryLow){
			//TODO - play roverBattLow.aac
		}
	}

	start();
}

BatteryLowService::~BatteryLowService(){
	stop(0);
	queue.unblock();

	while(running()){
		delayMillis(1);
	}

	Events::unlisten(&queue);
}

void BatteryLowService::loop(){
	Event event{};
	if(!queue.get(event, portMAX_DELAY)) return;

	if(event.facility == Facility::Battery && event.data != nullptr){
		auto* data = (Battery::Event*) event.data;
		if(data->action == Battery::Event::LevelChange){
			if(data->level == Battery::VeryLow){
				//TODO - play roverBattLow.aac
			}
		}
	}

	free(event.data);
}
