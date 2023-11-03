#include "LEDService.h"
#include <esp_log.h>
#include "Devices/SingleExpanderLED.h"
#include "Devices/SinglePwmLED.h"
#include "Util/LEDBlinkFunction.h"
#include "Util/LEDBreatheFunction.h"
#include "Pins.hpp"

static const char *TAG = "LEDService";

const std::map<LED, std::tuple<gpio_num_t, ledc_channel_t, uint8_t>> LEDService::pwmMappings = {};

const std::map<LED, std::tuple<uint8_t, uint8_t>> LEDService::expanderMappings = {
		{LED::Camera,         {EXP_LED_CAM,           0xFF}},
		{LED::Rear,           {EXP_LED_REAR,          0xFF}},
		{LED::LeftMotor,      {EXP_LED_MOTOR_L,       0xFF}},
		{LED::RightMotor,     {EXP_LED_MOTOR_R,       0xFF}},
		{LED::Arm,            {EXP_LED_ARM,           0xFF}},
		{LED::LeftHeadlight,  {EXP_LED_FRONT_L,       0x20}},
		{LED::RightHeadlight, {EXP_LED_FRONT_R,       0x20}},
		{LED::StatusGreen,    {EXP_LED_STATUS_GREEN,  0x20}},
		{LED::StatusYellow,   {EXP_LED_STATUS_YELLOW, 0xFF}},
		{LED::StatusRed,      {EXP_LED_STATUS_RED,    0xFF}},
};

LEDService::LEDService(AW9523 &aw9523) : SleepyThreaded(10, "LEDService") {
	std::lock_guard lock(mutex);

	for (LED led = (LED) 0; (uint8_t) led < (uint8_t) LED::COUNT; led = (LED) ((uint8_t) led + 1)) {
		const bool isExpander = expanderMappings.contains(led);
		const bool isPwm = pwmMappings.contains(led);

		if (isExpander && isPwm) {
			ESP_LOGE(TAG, "LED %d is marked as both expander and PWM.", (uint8_t) led);
		} else if (isExpander) {
			std::tuple<uint8_t, uint8_t> ledData = expanderMappings.at(led);
			SingleLED *ledDevice = new SingleExpanderLED(aw9523, std::get<0>(ledData), std::get<1>(ledData));
			ledDevices[led] = ledDevice;
		} else {
			std::tuple<gpio_num_t, ledc_channel_t, uint8_t> ledData = pwmMappings.at(led);
			SingleLED *ledDevice = new SinglePwmLED(std::get<0>(ledData), std::get<1>(ledData), std::get<2>(ledData));
			ledDevices[led] = ledDevice;
		}
	}

	start();
}

LEDService::~LEDService() {
	std::lock_guard lock(mutex);

	ledFunctions.clear();

	for (auto led: ledDevices) {
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

void LEDService::blink(LED led, uint32_t count /*= 1*/, uint32_t period /*= 1000*/) {
	std::lock_guard lock(mutex);

	if (ledFunctions.contains(led)) {
		ledFunctions.erase(led);
	}

	if (!ledDevices.contains(led)) {
		ESP_LOGW(TAG, "LED %d is set to blink, but does not exist.", (uint8_t) led);
		return;
	}

	ledFunctions[led] = std::make_unique<LEDBlinkFunction>(*ledDevices[led], count, period);
}

void LEDService::breathe(LED led, uint32_t period /*= 1000*/) {
	std::lock_guard lock(mutex);

	if (ledFunctions.contains(led)) {
		ledFunctions.erase(led);
	}

	if (!ledDevices.contains(led)) {
		ESP_LOGW(TAG, "LED %d is set to breathe, but does not exist.", (uint8_t) led);
		return;
	}

	ledFunctions[led] = std::make_unique<LEDBreatheFunction>(*ledDevices[led], period);
}

void LEDService::sleepyLoop() {
	std::lock_guard lock(mutex);

	for (LED led = (LED) 0; (uint8_t) led < (uint8_t) LED::COUNT; led = (LED) ((uint8_t) led + 1)) {
		if (!ledFunctions.contains(led)) {
			continue;
		}

		ledFunctions[led]->loop();
	}
}
