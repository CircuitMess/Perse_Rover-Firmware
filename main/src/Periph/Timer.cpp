#include "Timer.h"
#include <esp_log.h>

#include <utility>

static const char* TAG = "Timer";

Timer::Timer(const char* name, uint32_t period, std::function<void()> ISR) : period(period*1000), ISR(std::move(ISR)){
	char timerName[32];
	sprintf(timerName, "Tmr-%s", name);

	esp_timer_create_args_t args = {
			.callback = interrupt,
			.arg = this,
			.dispatch_method = ESP_TIMER_TASK,
			.name = timerName,
			.skip_unhandled_events = true
	};
	ESP_ERROR_CHECK(esp_timer_create(&args, &timer));
}

Timer::~Timer(){
	stop();
	esp_timer_delete(timer);
}

void IRAM_ATTR Timer::start(){
	esp_timer_start_once(timer, period);
}

void IRAM_ATTR Timer::stop(){
	esp_timer_stop(timer);
}

void Timer::reset(){
	esp_timer_stop(timer);
	esp_timer_start_once(timer, period);

}

void IRAM_ATTR Timer::interrupt(void* arg){
	auto timer = (Timer*) arg;
	timer->ISR();
}
