#include "Battery.h"
#include <cmath>
#include <utility>
#include "Pins.hpp"
#include "Util/Events.h"
#include "Util/stdafx.h"
#include "Services/Comm.h"
#include "Util/Services.h"
#include "Services/Audio.h"

#define MAX_READ 4600    // 4.6V
#define MIN_READ 3600    // 3.6V
#define FACTOR2 0.0000535803f
#define FACTOR1 1.3658f
#define FACTOR0 526.332f

Battery::Battery(ADC& adc) : SleepyThreaded(MeasureIntverval, "Battery", 3 * 1024, 5, 1),
							 adc(adc, (gpio_num_t) PIN_BATT, 0.05, MIN_READ, MAX_READ, (float) getVoltOffset() + FACTOR0, FACTOR1, FACTOR2),
							 hysteresis({ 0, 4, 15, 30, 70, 100 }, 3), eventQueue(10){
	Events::listen(Facility::TCP, &eventQueue);

	adc_unit_t unit;
	adc_channel_t chan;
	adc_oneshot_io_to_channel((gpio_num_t)BATTERY_ADC, &unit, &chan);

	adc.config(chan, {
			.atten = ADC_ATTEN_DB_11,
			.bitwidth = ADC_BITWIDTH_12
	});
	sample(true);
}

Battery::~Battery(){
	Events::unlisten(&eventQueue);
}

void Battery::begin() {
	start();
}

uint8_t Battery::getPerc() const {
	return adc.getValue();
}

Battery::Level Battery::getLevel() const {
	return (Level)hysteresis.get();
}

int16_t Battery::getVoltOffset() {
	int16_t upper = 0, lower = 0;
	ESP_ERROR_CHECK(esp_efuse_read_field_blob((const esp_efuse_desc_t**) EfuseAdcLow, &lower, 8));
	ESP_ERROR_CHECK(esp_efuse_read_field_blob((const esp_efuse_desc_t**) EfuseAdcHigh, &upper, 8));

	return (upper << 8) | lower;
}

uint16_t Battery::mapRawReading(uint16_t reading) {
	const float fval = reading;
	return std::round(FACTOR0 + fval * FACTOR1 + std::pow(fval, 2.0f) * FACTOR2);
}

bool Battery::isShutdown() const {
	return shutdown;
}

void Battery::setShutdownCallback(std::function<void()> callback){
	shutdownCallback = std::move(callback);
	if(!shutdownCallback || !shutdown) return;

	shutdownCallback();
}

void Battery::sleepyLoop() {
	if (shutdown) {
		return;
	}

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

	sample();

	if(!shouldSendState){
		return;
	}

	const uint8_t newValue = getPerc();

	if(newValue == oldValueSent){
		return;
	}

	Comm* comm = (Comm*) Services.get(Service::Comm);
	if(comm == nullptr){
		return;
	}

	comm->sendBattery(newValue);
	oldValueSent = newValue;
}

void Battery::sample(bool fresh/* = false*/) {
	if (shutdown) {
		return;
	}

	const Level oldLevel = getLevel();

	if (fresh) {
		adc.resetEma();
		hysteresis.reset(adc.getValue());
	}
	else {
		float val = adc.sample();
		hysteresis.update(val);
	}

	if (oldLevel != getLevel() || fresh) {
		Events::post(Facility::Battery, Battery::Event{.action = Event::LevelChange, .level = getLevel()});
	}

	if (getLevel() == Critical) {
		stop(0);
		shutdown = true;
		if(!shutdownCallback) return;
		shutdownCallback();
		return;
	}
}
