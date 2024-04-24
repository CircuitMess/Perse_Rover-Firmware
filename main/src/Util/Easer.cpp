#include "Easer.h"

Easer::Easer(const char* name, int32_t step, uint32_t period, std::function<void(int32_t)> cb) : timer(name, period, [this](){ timerFn(); }), step(step), cb(std::move(cb)){

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
