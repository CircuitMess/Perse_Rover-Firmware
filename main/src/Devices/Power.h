#ifndef PERSE_ROVER_POWER_H
#define PERSE_ROVER_POWER_H

/**
 * The Curiosity board has a latching power supply. Pressing the power button turns the load switch
 * on long enough for the ESP to boot; from then on the rover only stays powered while PIN_PWR_HOLD
 * is driven high. Releasing it cuts V_SYS, which is the only real way to turn the rover off.
 */
class Power {
public:
	/** Drives the power latch. Call as early as possible in app_main(). */
	static void hold();

	/** Releases the power latch. Does not return - the board loses power. */
	[[noreturn]] static void off();
};

#endif //PERSE_ROVER_POWER_H
