#ifndef PERSE_ROVER_BATTERY_H
#define PERSE_ROVER_BATTERY_H

#include <hal/gpio_types.h>
#include <esp_efuse.h>
#include "Util/Threaded.h"
#include "Util/Hysteresis.h"
#include "Services/ADCReader.h"

class Battery : private SleepyThreaded
{
public:
	enum Level { Critical = 0, VeryLow, Low, Mid, Full, COUNT };

	struct Event {
		enum {
			LevelChange
		} action;
		union {
			Level level;
		};
	};

public:
	Battery(ADC& adc);
	void begin();

	uint8_t getPerc() const;
	Level getLevel() const;

	static int16_t getVoltOffset();
	static uint16_t mapRawReading(uint16_t reading);

	bool isShutdown() const;

private:
	static constexpr uint32_t MeasureIntverval = 100;
	static constexpr esp_efuse_desc_t AdcLow = {EFUSE_BLK3, 0, 8 };
	static constexpr const esp_efuse_desc_t* EfuseAdcLow[] = {&AdcLow, nullptr };
	static constexpr esp_efuse_desc_t AdcHigh = {EFUSE_BLK3, 8, 8 };
	static constexpr const esp_efuse_desc_t* EfuseAdcHigh[] = {&AdcHigh, nullptr };

	ADCReader adc;
	Hysteresis hysteresis;
	bool shutdown = false;

private:
	void sleepyLoop() override;
	void sample(bool fresh = false);
};

#endif //PERSE_ROVER_BATTERY_H