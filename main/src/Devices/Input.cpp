#include "Input.h"
#include "Util/Events.h"
#include <Util/stdafx.h>
#include <Pins.hpp>
#include <driver/gpio.h>

// button index -> AW9523 IO port
const std::unordered_map<Input::Button, uint8_t> Input::PinMap{
		{ Pair, EXP_BTN_PAIR }
};

const std::unordered_map<Input::Button, const char*> Input::PinLabels{
		{ Pair, "Pair" }
};

Input::Input(AW9523& aw9523) : Threaded("Input", 2048, 6), aw9523(aw9523){
	for(const auto& pair : PinMap){
		const auto port = pair.first;
		const auto pin = pair.second;

		btnState[port] = false;
		dbTime[port] = 0;

		aw9523.pinMode(pin, AW9523::IN);
	}

	start();
}

Input::~Input(){
	stop();
}

void Input::loop(){
	scan();
	vTaskDelay(SleepTime);
}

void Input::scan(){
	for(const auto& pair: PinMap){
		const auto port = pair.first;
		const auto pin = pair.second;

		bool state = aw9523.read(pin);

		if(state){
			released(port);
		}else{
			pressed(port);
		}
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

	Data data = {
			.btn = btn,
			.action = Data::Release
	};
	Events::post(Facility::Input, data);
}
