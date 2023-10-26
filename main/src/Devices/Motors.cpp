#include "Motors.h"
#include <algorithm>
#include <cmath>
#include "Pins.hpp"
#include "Util/stdafx.h"

MotorControl::MotorControl(const std::array<ledc_channel_t, 2>& pwmChannels) :
		Threaded("Motors", 4 * 1024),
		pwm({ MOTOR_LEFT_A, pwmChannels[0] }, { MOTOR_RIGHT_A, pwmChannels[1] }),
		digitalPins({ MOTOR_LEFT_B }, { MOTOR_RIGHT_B }){
	begin();
}

void MotorControl::begin(){
	for(auto& pin : digitalPins){
		pin.on();
	}

	for(auto& pin : pwm){
		pin.setDuty(100);
	}

	stopAll();
	for(int i = 0; i < 4; i++){
		sendMotorPWM((Motor) i, 0);
	}
	start();
	easeCounter = micros();
}

void MotorControl::end(){
	for(auto& pin : digitalPins){
		pin.off();
	}

	for(auto& pin : pwm){
		pin.setDuty(0);
		pin.detach();
	}

	stop();
}

void MotorControl::setRight(int8_t value){
	sendMotorPWM(Right, value);
}

void MotorControl::setLeft(int8_t value){
	sendMotorPWM(Left, value);
}

void MotorControl::setAll(int8_t value){
	setAll({ value, value });
}

void MotorControl::setAll(MotorInfo state){
	setLeft(state.left);
	setRight(state.right);
}

MotorInfo MotorControl::getAll() const{
	return stateTarget.val;
}

void MotorControl::stopAll(){
	setAll({ 0, 0 });
}

void MotorControl::setMotorTarget(Motor motor, int8_t value){
	size_t i = motor;
	if(i >= 2) return;

	value = std::clamp(value, (int8_t) -100, (int8_t) 100);
	stateTarget.raw[i] = value;
}

void MotorControl::sendMotorPWM(Motor motor, int8_t value){
	size_t i = motor;
	if(i >= 2) return;

	const auto& pins = Pins[i];

	if(value == 0){
		pwm[i].detach();
		gpio_set_level(pins.first, 1);
		digitalPins[i].on();
		return;
	}

	// motor driver is a basic H bridge and has two input lines (A and B) for each motor
	// Ab - forward, aB - backward, AB - brake, ab - no power
	// going forward: pull B low, drive A with PWM
	// going backward: put B high, drive A with reverse PWM value

	// value is [-100, 100], duty is [0, 100]
	const bool reverse = (value < 0);
	uint8_t duty = abs(value);

	if(reverse){
		digitalPins[i].on();
		duty = 100 - duty;
	}else{
		digitalPins[i].off();
	}

	pwm[i].setDuty(duty);
}

void MotorControl::loop(){
	const auto elapsed = micros() - easeCounter;
	if(elapsed < EaseInterval){
		vTaskDelay(1);
		return;
	}
	easeCounter = micros();

	const double dt = (double) EaseInterval / 1000000.0;

	for(int i = 0; i < 2; i++){
		if(stateTarget.raw[i] == stateActual[i]) continue;
		double direction = (stateTarget.raw[i] > stateActual[i]) ? 1 : -1;

		double oldValue = stateActual[i];
		double newValue = std::clamp(oldValue + direction * EaseValue * dt, -100.0, 100.0);

		if(direction > 0){
			newValue = std::min(newValue, (double) stateTarget.raw[i]);
		}else{
			newValue = std::max(newValue, (double) stateTarget.raw[i]);
		}

		stateActual[i] = newValue;

		if(std::round(newValue) != std::round(oldValue)){
			sendMotorPWM((Motor) i, (int8_t) std::round(newValue));
		}
	}
}
