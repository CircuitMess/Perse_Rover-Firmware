#ifndef PERSE_ROVER_MCPWM_H
#define PERSE_ROVER_MCPWM_H

#include <hal/gpio_types.h>
#include "driver/mcpwm_prelude.h"

/**
 * Motor control PWM abstraction for DC motors, with speed and drive modes selection
 */
class MCPWM {
public:
	enum DriveMode{
		Forward, Reverse, Coast, Brake, None
	};
	MCPWM(gpio_num_t gpioA, gpio_num_t gpioB, int groupID); //groupID is 0 or 1
	virtual ~MCPWM();
	void enable();
	void disable();
	void setSpeed(uint8_t speed); //0 - 100
	void setMode(DriveMode mode);

private:
	static constexpr uint32_t TimerResolution = 10000000; // 10MHz, 1 tick = 0.1us
	static constexpr uint32_t PWMResolution = 25000;    // 25KHz PWM
	static constexpr uint32_t MaxDuty = (TimerResolution / PWMResolution); // maximum value we can set for the duty cycle, in ticks

	mcpwm_timer_handle_t timerHandle;
	mcpwm_oper_handle_t operatorHandle;
	mcpwm_cmpr_handle_t compA;
	mcpwm_cmpr_handle_t compB;
	mcpwm_gen_handle_t genA;
	mcpwm_gen_handle_t genB;
};


#endif //PERSE_ROVER_MCPWM_H
