#ifndef PERSE_ROVER_BATTERY_H
#define PERSE_ROVER_BATTERY_H

#include <hal/gpio_types.h>
#include "Util/Threaded.h"
#include "Devices/TCA9555.h"
#include "Util/Events.h"

/**
 * Battery monitoring for the Curiosity v0.4 board.
 *
 * HARDWARE LIMITATION: this board has no battery ADC. The 1:4 divider off the battery
 * (R58/R60/R61, with a TL431 reference switched in by TCA_CALIB_EN) ends on TCA_BATT_READ, which is
 * a *digital* input of the XL9555 expander instead of an ADC-capable GPIO of the ESP32. A quarter of
 * a single-cell voltage never crosses the expander's input threshold, so the tap cannot report a
 * level - only whatever the threshold happens to do. IO6, the pin Perseverance used for exactly this
 * measurement, is left unconnected on v0.4.
 *
 * Until that divider is routed to an ADC pin, this class reports a full battery and never triggers
 * the low-battery shutdown; the only real information available is the TP4056 charger state.
 * The divider is still sampled once at startup and logged, so the tap can be checked on hardware.
 */
class Battery : private SleepyThreaded
{
public:
	enum Level { Critical = 0, VeryLow, Low, Mid, Full, COUNT };

	enum class ChargingState { Unplugged, Charging, Charged };

	struct Event {
		enum {
			LevelChange
		} action;
		union {
			Level level;
		};
	};

public:
	explicit Battery(TCA9555& tca);
	virtual ~Battery();

	void begin();

	uint8_t getPerc() const;
	Level getLevel() const;

	ChargingState getChargingState() const;

	bool isShutdown() const;

	void setShutdownCallback(std::function<void()> callback);

private:
	static constexpr uint32_t MeasureIntverval = 500;

	TCA9555& tca;
	ChargingState chargingState = ChargingState::Unplugged;
	EventQueue eventQueue;
	bool shouldSendState = false;
	uint8_t oldValueSent = 0;
	std::function<void()> shutdownCallback = {};

private:
	void sleepyLoop() override;
	void sampleCharging();
	void logDividerTap();
};

#endif //PERSE_ROVER_BATTERY_H
