#ifndef PERSE_ROVER_MOTORS_H
#define PERSE_ROVER_MOTORS_H

#include <array>
#include "Pins.hpp"
#include "Periph/PWM.h"
#include "Periph/PinOut.h"
#include "Util/Threaded.h"

struct MotorInfo {
	int8_t frontLeft;
	int8_t frontRight;
	int8_t backLeft;
	int8_t backRight;
};

class MotorControl : private Threaded {
public:
	MotorControl(const std::array<ledc_channel_t, 4>& pwmChannels);
	void begin();
	void end();

	//motor values between -100 and 100
	void setFR(int8_t value);
	void setFL(int8_t value);
	void setBR(int8_t value);
	void setBL(int8_t value);
	void setRight(int8_t value);
	void setLeft(int8_t value);
	void setAll(int8_t value);


	void setAll(MotorInfo state);
	MotorInfo getAll() const;

	void stopAll();

private:
	enum Motor : uint8_t {
		FrontLeft, FrontRight, BackLeft, BackRight
	};

	PWM pwm[4]; //for A pins
	PinOut digitalPins[4]; //for B pins

	static constexpr std::pair<gpio_num_t, gpio_num_t> Pins[4] = {
			{ (gpio_num_t)MOTOR_FL_A, (gpio_num_t)MOTOR_FL_B },
			{ (gpio_num_t)MOTOR_FR_A, (gpio_num_t)MOTOR_FR_B },
			{ (gpio_num_t)MOTOR_BL_A, (gpio_num_t)MOTOR_BL_B },
			{ (gpio_num_t)MOTOR_BR_B, (gpio_num_t)MOTOR_BR_A }
	};

	union {
		MotorInfo val;
		int8_t raw[4];
	} stateTarget = { .val = { 0, 0, 0, 0 } };

	double stateActual[4] = { 0, 0, 0, 0 };

	void setMotorTarget(Motor motor, int8_t value);
	void sendMotorPWM(Motor motor, int8_t value);

	static constexpr double EaseValue = 200; // value change per second
	static constexpr uint32_t EaseInterval = 10000; // adjust PWM value every EaseInterval microseconds
	uint32_t easeCounter = 0;

	void loop() override;

};


#endif //PERSE_ROVER_MOTORS_H
