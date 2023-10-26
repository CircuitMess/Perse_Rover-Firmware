#ifndef PERSE_ROVER_STATEMACHINE_H
#define PERSE_ROVER_STATEMACHINE_H

#include <vector>
#include <string>
#include "Util/Threaded.h"

class State
{
public:
	State() = default;
	virtual ~State() = default;

	virtual void loop() {}
};

class StateMachine : private Threaded
{
public:
	StateMachine();
	virtual ~StateMachine() = default;

	void begin();

	template<typename T, typename... Args>
	State* transition(Args&&... args);

protected:
	virtual void loop() override;

private:
	State* currentState = nullptr;
};

template<typename T, typename... Args>
State* StateMachine::transition(Args&&... args) {
	State* nextState = new T(args...);

	delete currentState;
	currentState = nextState;
	return currentState;
}

#endif //PERSE_ROVER_STATEMACHINE_H