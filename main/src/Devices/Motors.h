#ifndef PERSE_ROVER_MOTORS_H
#define PERSE_ROVER_MOTORS_H

#include <array>
#include "Pins.hpp"
#include "Periph/PWM.h"
#include "Periph/PinOut.h"
#include "Util/Threaded.h"
#include "DriveInfo.h"

class MotorControl : private Threaded {
public:
	MotorControl(const std::array<ledc_channel_t, 2>& pwmChannels);
	void begin();
	void end();

	//motor values between -100 and 100
	void setRight(int8_t value);
	void setLeft(int8_t value);
	void setAll(int8_t value);


	void setAll(MotorInfo state);
	MotorInfo getAll() const;

	void stopAll();

private:
	enum Motor : uint8_t {
		Left, Right
	};

	PWM pwm[2]; //for A pins
	PinOut digitalPins[2]; //for B pins

	static constexpr std::pair<gpio_num_t, gpio_num_t> Pins[2] = {
			{ (gpio_num_t) MOTOR_LEFT_A,  (gpio_num_t) MOTOR_LEFT_B },
			{ (gpio_num_t) MOTOR_RIGHT_A, (gpio_num_t) MOTOR_RIGHT_B }
	};

	union {
		MotorInfo val;
		int8_t raw[2];
	} stateTarget = { .val = { 0, 0 } };

	double stateActual[2] = { 0, 0 };

	void setMotorTarget(Motor motor, int8_t value);
	void sendMotorPWM(Motor motor, int8_t value);

	static constexpr double EaseValue = 200; // value change per second
	static constexpr uint32_t EaseInterval = 10000; // adjust PWM value every EaseInterval microseconds
	uint32_t easeCounter = 0;

	void loop() override;

};


#endif //PERSE_ROVER_MOTORS_H
