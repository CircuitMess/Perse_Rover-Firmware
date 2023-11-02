#include "LEDService.h"
#include <esp_log.h>
#include "Devices/SingleExpanderLED.h"
#include "Devices/SinglePwmLED.h"
#include "Util/LEDBlinkFunction.h"
#include "Util/LEDBreatheFunction.h"

static const char* TAG = "LEDService";

LEDService::LEDService(AW9523& aw9523) : SleepyThreaded(10, "LEDService") {
	std::lock_guard lock(mutex);

	for (LED led = (LED)0; (uint8_t)led < (uint8_t)LED::COUNT; led = (LED)((uint8_t)led + 1)) {
		const bool isExpander = expanderMappings.contains(led);
		const bool isPwm = pwmMappings.contains(led);

		if (isExpander && isPwm) {
			ESP_LOGE(TAG, "LED %d is marked as both expander and PWM.", (uint8_t)led);
		}
		else if (isExpander) {
			std::tuple<uint8_t, uint8_t> ledData = expanderMappings.at(led);
			SingleLED* ledDevice = new SingleExpanderLED(aw9523, std::get<0>(ledData), std::get<1>(ledData));
			ledDevices[led] = ledDevice;
		}
		else {
			std::tuple<gpio_num_t, ledc_channel_t, uint8_t> ledData = pwmMappings.at(led);
			SingleLED* ledDevice = new SinglePwmLED(std::get<0>(ledData), std::get<1>(ledData), std::get<2>(ledData));
			ledDevices[led] = ledDevice;
		}
	}
}

LEDService::~LEDService() {
	std::lock_guard lock(mutex);

	ledFunctions.clear();

	for (auto led : ledDevices) {
		delete led.second;
	}
}

void LEDService::on(LED led) {
	std::lock_guard lock(mutex);

	if (ledFunctions.contains(led)) {
		ledFunctions.erase(led);
	}

	if (!ledDevices.contains(led)) {
		return;
	}

	if (ledDevices[led] == nullptr) {
		return;
	}

	ledDevices[led]->setValue(0xFF);
}

void LEDService::off(LED led) {
	std::lock_guard lock(mutex);

	if (ledFunctions.contains(led)) {
		ledFunctions.erase(led);
	}

	if (!ledDevices.contains(led)) {
		return;
	}

	if (ledDevices[led] == nullptr) {
		return;
	}

	ledDevices[led]->setValue(0);
}

void LEDService::blink(LED led, uint32_t count /*= 1*/, uint32_t period /*= 500*/) {
	std::lock_guard lock(mutex);

	if (ledFunctions.contains(led)) {
		ledFunctions.erase(led);
	}

	if (!ledDevices.contains(led)) {
		ESP_LOGW(TAG, "LED %d is set to blink, but does not exist.", (uint8_t)led);
		return;
	}

	ledFunctions[led] = std::make_unique<LEDBlinkFunction>(*ledDevices[led], count, period /*= 500*/);
}

void LEDService::breathe(LED led, uint32_t period /*= 500*/) {
	std::lock_guard lock(mutex);

	if (ledFunctions.contains(led)) {
		ledFunctions.erase(led);
	}

	if (!ledDevices.contains(led)) {
		ESP_LOGW(TAG, "LED %d is set to breathe, but does not exist.", (uint8_t)led);
		return;
	}

	ledFunctions[led] = std::make_unique<LEDBreatheFunction>(*ledDevices[led], period);
}

void LEDService::sleepyLoop() {
	std::lock_guard lock(mutex);

	for (LED led = (LED)0; (uint8_t)led < (uint8_t)LED::COUNT; led = (LED)((uint8_t)led + 1)) {
		if (!ledFunctions.contains(led)) {
			continue;
		}

		ledFunctions[led]->loop();
	}
}
