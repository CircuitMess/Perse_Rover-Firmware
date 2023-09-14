#ifndef PERSE_ROVER_SERVO_H
#define PERSE_ROVER_SERVO_H

#include "driver/mcpwm_prelude.h"
#include <hal/gpio_types.h>
#include "Util/Threaded.h"

class Servo {
public:
	Servo(gpio_num_t gpio, int groupID); //groupID is 0 or 1
	~Servo();
	void enable();
	void disable();
	void setValue(uint8_t value); //0 - 100, 0 is leftmost, 100 is rightmost

private:
	mcpwm_timer_handle_t timerHandle;
	mcpwm_oper_handle_t operatorHandle;
	mcpwm_cmpr_handle_t compHandle;
	mcpwm_gen_handle_t genHandle;


	static constexpr uint32_t angle_to_comparator_value(int value);

	//taken from datasheet (AND ADJUSTED): https://www.electronicoscaldas.com/datasheet/MG90S_Tower-Pro.pdf
	static constexpr uint32_t MinPulseWidth = 500;  // Minimum pulse width in microsecond
	static constexpr uint32_t MaxPulseWidth = 2500;  // Maximum pulse width in microsecond

	static constexpr uint32_t TimerResolution = 1000000;  // 1MHz, 1us per tick
	static constexpr uint32_t PeriodLength = 20000;    // 20000 ticks, 20ms
};


#endif //PERSE_ROVER_SERVO_H
