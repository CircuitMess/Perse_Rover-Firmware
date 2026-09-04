#include "Input.h"
#include "Util/Events.h"
#include <Util/stdafx.h>
#include <Pins.hpp>
#include <driver/gpio.h>

// button index -> GPIO
const std::unordered_map<Input::Button, uint8_t> Input::PinMap{
		{ Power, PIN_BTN }
};

const std::unordered_map<Input::Button, const char*> Input::PinLabels{
		{ Power, "Power" }
};

Input::Input() : Threaded("Input", 2048, 6){
	for(const auto& pair : PinMap){
		const auto port = pair.first;
		const auto pin = pair.second;

		const gpio_config_t cfg = {
				.pin_bit_mask = 1ULL << pin,
				.mode = GPIO_MODE_INPUT,
				.pull_up_en = GPIO_PULLUP_DISABLE,
				.pull_down_en = GPIO_PULLDOWN_DISABLE, // R64 pulls the divider down on the board
				.intr_type = GPIO_INTR_DISABLE
		};
		gpio_config(&cfg);

		/* The rover is turned on by holding the power button, so it is usually still down when we get
		 * here. Adopt that as the starting state and mark the hold as already handled, otherwise the
		 * press that powered the rover on would immediately be read as a request to power it off. */
		btnState[port] = gpio_get_level((gpio_num_t) pin);
		dbTime[port] = 0;
		pressTime[port] = millis();
		holdSent[port] = btnState[port];
	}

	start();
}

Input::~Input(){
	stop();
}

bool Input::getState(Input::Button btn){
	if(!btnState.contains(btn)) return false;
	return btnState.at(btn);
}

void Input::loop(){
	scan();
	vTaskDelay(SleepTime);
}

void Input::scan(){
	for(const auto& pair: PinMap){
		const auto port = pair.first;
		const auto pin = pair.second;

		// The divider pulls PIN_BTN high while the button is pressed.
		const bool state = gpio_get_level((gpio_num_t) pin);

		if(state){
			pressed(port);
		}else{
			released(port);
		}

		if(!btnState[port] || holdSent[port]) continue;

		if(millis() - pressTime[port] < HoldTime) continue;

		holdSent[port] = true;

		Data data = {
				.btn = port,
				.action = Data::Hold
		};
		Events::post(Facility::Input, data);
	}
}

void Input::pressed(Input::Button btn){
	if(btnState[btn]){
		dbTime[btn] = 0;
		return;
	}

	auto t = millis();

	if(dbTime[btn] == 0){
		dbTime[btn] = t;
		return;
	}else if(t - dbTime[btn] < DebounceTime){
		return;
	}

	btnState[btn] = true;
	dbTime[btn] = 0;
	pressTime[btn] = t;
	holdSent[btn] = false;

	Data data = {
			.btn = btn,
			.action = Data::Press
	};
	Events::post(Facility::Input, data);
}

void Input::released(Input::Button btn){
	if(!btnState[btn]){
		dbTime[btn] = 0;
		return;
	}

	auto t = millis();

	if(dbTime[btn] == 0){
		dbTime[btn] = t;
		return;
	}else if(t - dbTime[btn] < DebounceTime){
		return;
	}

	btnState[btn] = false;
	dbTime[btn] = 0;
	holdSent[btn] = false;

	Data data = {
			.btn = btn,
			.action = Data::Release
	};
	Events::post(Facility::Input, data);
}
