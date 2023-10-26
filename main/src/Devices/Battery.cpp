#include "Battery.h"
#include <cmath>
#include "Pins.hpp"
#include "Util/Events.h"
#include "Util/stdafx.h"

#define MAX_READ 2724	// 4.5V
#define MIN_READ 2280	// 3.8V

Battery::Battery(ADC& adc) : SleepyThreaded(MeasureIntverval, "Battery", 3 * 1024, 5, 1),
					 adc(adc, (gpio_num_t)BATTERY_ADC, 0.05, MIN_READ, MAX_READ, getVoltOffset()),
					 hysteresis({ 0, 4, 15, 30, 70, 100 }, 3) {

	adc_unit_t unit;
	adc_channel_t chan;
	adc_oneshot_io_to_channel((gpio_num_t)BATTERY_ADC, &unit, &chan);

	adc.config(chan, {
			.atten = ADC_ATTEN_DB_11,
			.bitwidth = ADC_BITWIDTH_12
	});
	sample(true);
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
	return std::round(map((double)reading, MIN_READ, MAX_READ, 3800, 4500));
}

bool Battery::isShutdown() const {
	return shutdown;
}

void Battery::sleepyLoop() {
	if (shutdown) {
		return;
	}

	sample();
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
		return;
	}
}
