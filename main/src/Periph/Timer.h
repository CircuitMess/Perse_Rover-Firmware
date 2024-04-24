#ifndef CLOCKSTAR_FIRMWARE_TIMER_H
#define CLOCKSTAR_FIRMWARE_TIMER_H

#include <esp_attr.h>
#include <esp_timer.h>
#include <functional>

/**
 * ESP Timer HAL takes care of HW allocation when Timers are created. (No need to specify timerID and timerGroup)
 * The HW has 4 timers, so construction of more will fail.
 */

class Timer {
public:
	/**
	 * @param period - Time between ISR calls [ms]
	 * @param ISR - ISR routine to be called when period time runs out. Must be non-blocking (using fromISR functions)
	 * @param dataPtr - data pointer passed to ISR routine, optional
	 */
	Timer(const char* name, uint32_t period, std::function<void()> ISR);
	virtual ~Timer();

	void start();
	void stop();
	void reset();

private:
	static void interrupt(void* arg);
	esp_timer_handle_t timer;

	uint64_t period;
	const std::function<void()> ISR;

};


#endif //CLOCKSTAR_FIRMWARE_TIMER_H
