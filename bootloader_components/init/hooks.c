#include <hal/gpio_hal.h>

/* The Curiosity board latches its own power: pressing the power button turns the load switch on,
 * and the rover only stays on while PWR_HOLD keeps the latch driven. Assert it here, in the
 * bootloader, so the user only has to hold the button for a few milliseconds instead of for the
 * whole boot. app_main() re-asserts it through the regular GPIO driver. */
#define PIN_PWR_HOLD GPIO_NUM_45

/* Function used to tell the linker to include this file with all its symbols. */
void bootloader_hooks_include(void){
}

void bootloader_before_init(void){
	gpio_ll_output_enable(&GPIO, PIN_PWR_HOLD);
	gpio_ll_pulldown_dis(&GPIO, PIN_PWR_HOLD);
	gpio_ll_pullup_dis(&GPIO, PIN_PWR_HOLD);
	gpio_ll_set_level(&GPIO, PIN_PWR_HOLD, 1);
}

void bootloader_after_init(void){
}
