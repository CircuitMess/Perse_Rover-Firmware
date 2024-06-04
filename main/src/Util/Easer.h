#ifndef PERSE_ROVER_EASER_H
#define PERSE_ROVER_EASER_H


#include "Periph/Timer.h"
#include "Threaded.h"

class Easer {
public:
	Easer(const char* name, int32_t step, uint32_t period, std::function<void(int32_t)> cb);

	void set(int32_t target);

private:
	Timer timer;

	int32_t step;
	uint32_t period;
	std::function<void(int32_t)> cb;

	void timerFn();

	bool isSet = false;
	int32_t current;
	int32_t target;

	void process();
	static QueueHandle_t easerQueue; //queue of ptrs that need to be processed
	static ThreadedClosure easerTask;

};


#endif //PERSE_ROVER_EASER_H
