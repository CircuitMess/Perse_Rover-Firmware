#include "MCPWM.h"
#include "driver/mcpwm_timer.h"

MCPWM::MCPWM(gpio_num_t gpioA, gpio_num_t gpioB, int groupID){
	// mcpwm timer
	const mcpwm_timer_config_t timer_config = {
			.group_id = groupID,
			.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
			.resolution_hz = TimerResolution,
			.count_mode = MCPWM_TIMER_COUNT_MODE_UP,
			.period_ticks = MaxDuty,
	};
	ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timerHandle));

	const mcpwm_operator_config_t operator_config = {
			.group_id = groupID,
	};

	mcpwm_new_operator(&operator_config, &operatorHandle);

	mcpwm_operator_connect_timer(operatorHandle, timerHandle);

	const mcpwm_comparator_config_t comparator_config = {
			.flags = { .update_cmp_on_tez = true },
	};
	mcpwm_new_comparator(operatorHandle, &comparator_config, &compA);
	mcpwm_new_comparator(operatorHandle, &comparator_config, &compB);

	// set the initial compare value for both comparators
	mcpwm_comparator_set_compare_value(compA, 0);
	mcpwm_comparator_set_compare_value(compB, 0);

	mcpwm_generator_config_t generator_config = {
			.gen_gpio_num = gpioA,
	};
	mcpwm_new_generator(operatorHandle, &generator_config, &genA);
	generator_config.gen_gpio_num = gpioB;
	mcpwm_new_generator(operatorHandle, &generator_config, &genB);

	mcpwm_generator_set_actions_on_timer_event(genA,
											   MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
											   MCPWM_GEN_TIMER_EVENT_ACTION_END());
	mcpwm_generator_set_actions_on_compare_event(genA,
												 MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, compA, MCPWM_GEN_ACTION_LOW),
												 MCPWM_GEN_COMPARE_EVENT_ACTION_END());
	mcpwm_generator_set_actions_on_timer_event(genB,
											   MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
											   MCPWM_GEN_TIMER_EVENT_ACTION_END());
	mcpwm_generator_set_actions_on_compare_event(genB,
												 MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, compB, MCPWM_GEN_ACTION_LOW),
												 MCPWM_GEN_COMPARE_EVENT_ACTION_END());

}


MCPWM::~MCPWM(){
	mcpwm_del_generator(genA);
	mcpwm_del_generator(genB);
	mcpwm_del_comparator(compA);
	mcpwm_del_comparator(compB);
	mcpwm_del_operator(operatorHandle);
	mcpwm_del_timer(timerHandle);
}


void MCPWM::setSpeed(uint8_t speed){
	const uint32_t val = MaxDuty * speed / 100;
	mcpwm_comparator_set_compare_value(compA, val);
	mcpwm_comparator_set_compare_value(compB, val);

}

void MCPWM::enable(){
	mcpwm_timer_enable(timerHandle);
	mcpwm_timer_start_stop(timerHandle, MCPWM_TIMER_START_NO_STOP);

}

void MCPWM::disable(){
	mcpwm_timer_start_stop(timerHandle, MCPWM_TIMER_STOP_EMPTY);
	mcpwm_timer_disable(timerHandle);

}

void MCPWM::setMode(MCPWM::DriveMode mode){
	switch(mode){
		case Forward:
			mcpwm_generator_set_force_level(genA, -1, true);
			mcpwm_generator_set_force_level(genB, 0, true);

			break;
		case Reverse:
			mcpwm_generator_set_force_level(genB, -1, true);
			mcpwm_generator_set_force_level(genA, 0, true);

			break;
		case Coast:
			mcpwm_generator_set_force_level(genA, 0, true);
			mcpwm_generator_set_force_level(genB, 0, true);

			break;
		case Brake:
			mcpwm_generator_set_force_level(genA, 1, true);
			mcpwm_generator_set_force_level(genB, 1, true);

			break;
		case None:
			disable();
			break;
	}
}
