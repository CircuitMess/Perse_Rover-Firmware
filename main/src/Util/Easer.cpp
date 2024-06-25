#include "Easer.h"

QueueHandle_t  Easer::easerQueue = xQueueCreate(24, sizeof(Easer*));
ThreadedClosure Easer::easerTask = ThreadedClosure([](){
	Easer* ptr;
	if(!xQueueReceive(Easer::easerQueue, &ptr, portMAX_DELAY)) return;

	if(ptr == nullptr) return;

	ptr->process();

}, "easerTask", 3 * 1024);

//TODO - abort task and clear queue when last Easer instance is destroyed

Easer::Easer(const char* name, int32_t step, uint32_t period, std::function<void(int32_t)> cb) : timer(name, period, [this](){ timerFn(); }), step(step),
																								 cb(std::move(cb)){
	if(!easerTask.running()){
		easerTask.start();
	}
}

void Easer::set(int32_t val){
	target = val;

	if(!isSet){
		current = val;
		cb(val);
		isSet = true;
		return;
	}

	if(current != target){
		timer.reset();
	}
}

void IRAM_ATTR Easer::timerFn(){
	BaseType_t priority = pdFALSE;
	Easer* ptr = this;
	xQueueSendFromISR(easerQueue, &ptr, &priority);
}

void Easer::process(){
	int32_t newCurrent = current;
	if(target > current){
		newCurrent += step;
	}else{
		newCurrent -= step;
	}

	if((target > current && newCurrent > target) || (target < current && newCurrent < target)){
		current = target;
	}else{
		current = newCurrent;
	}

	cb(current);

	if(current != target){
		timer.start();
	}
}
