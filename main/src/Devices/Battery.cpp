#include "Battery.h"
#include <utility>
#include <esp_log.h>
#include "Pins.hpp"
#include "Util/Events.h"
#include "Util/stdafx.h"
#include "Services/Comm.h"
#include "Util/Services.h"

static const char* TAG = "Battery";

Battery::Battery(TCA9555& tca) : SleepyThreaded(MeasureIntverval, "Battery", 3 * 1024, 5, 1),
								 tca(tca), eventQueue(10){
	Events::listen(Facility::TCP, &eventQueue);

	tca.pinMode(TCA_CHARGE, TCA9555::IN);
	tca.pinMode(TCA_STANDBY, TCA9555::IN);
	tca.pinMode(TCA_USB_DETECT, TCA9555::IN);
	tca.pinMode(TCA_BATT_READ, TCA9555::IN);
	tca.pinMode(TCA_CALIB_EN, TCA9555::OUT);
	tca.write(TCA_CALIB_EN, false);

	ESP_LOGW(TAG, "This board has no battery ADC - reporting a full battery. See Devices/Battery.h.");
	logDividerTap();

	sampleCharging();
}

Battery::~Battery(){
	Events::unlisten(&eventQueue);
}

void Battery::begin(){
	start();

	// Nothing else ever moves the level, so announce the one we have and be done with it.
	Events::post(Facility::Battery, Battery::Event{ .action = Event::LevelChange, .level = getLevel() });
}

uint8_t Battery::getPerc() const{
	return 100;
}

Battery::Level Battery::getLevel() const{
	return Full;
}

Battery::ChargingState Battery::getChargingState() const{
	return chargingState;
}

bool Battery::isShutdown() const{
	return false;
}

void Battery::setShutdownCallback(std::function<void()> callback){
	shutdownCallback = std::move(callback);
}

void Battery::sleepyLoop(){
	for(::Event event{}; eventQueue.get(event, 0);){
		if(event.facility != Facility::TCP){
			free(event.data);
			continue;
		}

		const TCPServer::Event* tcpEvent = (TCPServer::Event*) event.data;
		if(tcpEvent == nullptr){
			continue;
		}

		if(tcpEvent->status == TCPServer::Event::Status::Connected){
			shouldSendState = true;

			if(Comm* comm = (Comm*) Services.get(Service::Comm)){
				const uint8_t newValue = getPerc();
				comm->sendBattery(newValue);
				oldValueSent = newValue;
			}
		}else{
			shouldSendState = false;
			oldValueSent = 0;
		}

		free(event.data);
	}

	sampleCharging();

	if(!shouldSendState) return;

	const uint8_t newValue = getPerc();
	if(newValue == oldValueSent) return;

	Comm* comm = (Comm*) Services.get(Service::Comm);
	if(comm == nullptr) return;

	comm->sendBattery(newValue);
	oldValueSent = newValue;
}

void Battery::sampleCharging(){
	// Both TP4056 status pins are open drain and pulled low when active.
	const bool usb = tca.read(TCA_USB_DETECT);
	const bool charging = !tca.read(TCA_CHARGE);
	const bool standby = !tca.read(TCA_STANDBY);

	ChargingState newState;
	if(!usb){
		newState = ChargingState::Unplugged;
	}else if(charging){
		newState = ChargingState::Charging;
	}else if(standby){
		newState = ChargingState::Charged;
	}else{
		newState = ChargingState::Unplugged;
	}

	if(newState == chargingState) return;

	chargingState = newState;

	static const char* const names[] = { "unplugged", "charging", "charged" };
	ESP_LOGI(TAG, "Charger: %s", names[(int) chargingState]);
}

void Battery::logDividerTap(){
	/* The divider tap is a digital input, so all this can tell us is which side of the expander's
	 * input threshold the battery (and the 2.5V reference) lands on. Logged once so the tap can be
	 * checked against hardware without reflashing. */
	tca.write(TCA_CALIB_EN, false);
	delayMillis(10);
	const bool battTap = tca.read(TCA_BATT_READ);

	tca.write(TCA_CALIB_EN, true);
	delayMillis(10);
	const bool refTap = tca.read(TCA_BATT_READ);

	tca.write(TCA_CALIB_EN, false);

	ESP_LOGI(TAG, "Divider tap: battery=%d, 2.5V reference=%d", battTap, refTap);
}
