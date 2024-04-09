#include <esp_log.h>
#include "InactivityService.h"
#include "Util/stdafx.h"
#include "Comm.h"
#include "PairService.h"
#include "Audio.h"
#include "Util/Services.h"
#include "Devices/MotorDriveController.h"
#include "LEDService.h"

static const char* TAG = "InactivityService";

InactivityService::InactivityService() : Threaded("Inactivity", 2 * 1024), queue(24){
	Events::listen(Facility::Input, &queue);
	Events::listen(Facility::Pair, &queue);
	Events::listen(Facility::Comm, &queue);
	Events::listen(Facility::TCP, &queue);
	Events::listen(Facility::Modules, &queue);
	timer = millis();
	start();
}

void InactivityService::loop(){
	if(checkActions()){
		timer = millis();
		return;
	}

	if((paired && millis() - timer >= PairedTimeout) || (!paired && millis() - timer >= UnpairedTimeout)){
		Events::unlisten(&queue);
		ESP_LOGI(TAG, "Inactivity shutdown!");

		Audio* audio = (Audio*) Services.get(Service::Audio);
		delete audio;

		extern void shutdown();

		if(MotorDriveController* motors = (MotorDriveController*) Services.get(Service::MotorDriveController)){
			motors->setControl(Local);
			motors->setLocally({});
		}

		if(LEDService* led = (LEDService*) Services.get(Service::LED)){
			for(int i = 0; i < (uint8_t) LED::COUNT; i++){
				led->off((LED) i);
			}
		}

		delayMillis(1000);

		shutdown();
	}
}

bool InactivityService::checkActions(){
	Event e{};
	if(!queue.get(e, 1000)) return false;

	if(e.data == nullptr){
		return false;
	}

	bool action = false;
	switch(e.facility){
		case Facility::Comm:{
			auto* commData = (Comm::Event*) e.data;
			/* Note: this if() represents resetting the timeout for user-related commands from Comm,
			 * but it was easier to exclude only the status packets such as BatteryCritical, ConnectionStatus. */
			if(commData->type != CommType::None && commData->type != CommType::ControllerBatteryCritical && commData->type != CommType::ConnectionStrength){
				action = true;
				ESP_LOGD(TAG, "Comm action reset");
			}
			break;
		}
		case Facility::TCP:{
			auto* tcpData = (TCPServer::Event*) e.data;
			action = true;
			paired = tcpData->status == TCPServer::Event::Status::Connected;
			ESP_LOGD(TAG, "TCP action reset");
			break;
		}
		case Facility::Pair:{
			auto* pairData = (PairService::Event*) e.data;
			action = true;
			paired = pairData->success;
			ESP_LOGD(TAG, "Pair action reset");
			break;
		}
		case Facility::Input:{
			action = true;
			ESP_LOGD(TAG, "Input action reset");
			break;
		}
		case Facility::Modules:{
			action = true;
			ESP_LOGD(TAG, "Modules action reset");
			break;
		}
		default:
			break;
	}
	free(e.data);

	return action;
}
