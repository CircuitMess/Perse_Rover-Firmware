#include "Servo.h"
#include "driver/mcpwm_timer.h"
#include "Util/stdafx.h"

static const char* TAG = "Servo";

Servo::Servo(gpio_num_t gpio, int groupID){
	mcpwm_timer_config_t timer_config = {
			.group_id = groupID,
			.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
			.resolution_hz = TimerResolution,
			.count_mode = MCPWM_TIMER_COUNT_MODE_UP,
			.period_ticks = PeriodLength,
	};
	ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timerHandle));

	mcpwm_operator_config_t operator_config = {
			.group_id = 0, // operator must be in the same group to the timer
	};
	ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &operatorHandle));

	ESP_ERROR_CHECK(mcpwm_operator_connect_timer(operatorHandle, timerHandle));

	mcpwm_comparator_config_t comparator_config = {
			.flags = { .update_cmp_on_tez = true },
	};
	ESP_ERROR_CHECK(mcpwm_new_comparator(operatorHandle, &comparator_config, &compHandle));

	mcpwm_generator_config_t generator_config = {
			.gen_gpio_num = gpio,
	};
	ESP_ERROR_CHECK(mcpwm_new_generator(operatorHandle, &generator_config, &genHandle));

	// set the initial compare value, so that the servo will spin to the center position
	ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(compHandle, angle_to_comparator_value(50)));

	// go high on counter empty
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(genHandle,
															  MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY,
																						   MCPWM_GEN_ACTION_HIGH)));
	// go low on compare threshold
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(genHandle,
																MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, compHandle, MCPWM_GEN_ACTION_LOW)));

}


Servo::~Servo(){
	mcpwm_del_generator(genHandle);
	mcpwm_del_comparator(compHandle);
	mcpwm_del_operator(operatorHandle);
	mcpwm_del_timer(timerHandle);
}

void Servo::enable(){
	ESP_ERROR_CHECK(mcpwm_timer_enable(timerHandle));
	ESP_ERROR_CHECK(mcpwm_timer_start_stop(timerHandle, MCPWM_TIMER_START_NO_STOP));
}

void Servo::disable(){
	mcpwm_timer_start_stop(timerHandle, MCPWM_TIMER_STOP_EMPTY);
	mcpwm_timer_disable(timerHandle);
}

void Servo::setValue(uint8_t value){
	value = std::clamp((int) value, 0, 100);
	mcpwm_comparator_set_compare_value(compHandle, angle_to_comparator_value(value));
}


constexpr uint32_t Servo::angle_to_comparator_value(int value){
	uint32_t val = map((double) value, 0, 100, MinPulseWidth, MaxPulseWidth);
	return val;
}