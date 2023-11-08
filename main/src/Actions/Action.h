#ifndef PERSE_ROVER_ACTION_H
#define PERSE_ROVER_ACTION_H

class Action {
public:
	Action() = default;
	virtual ~Action() = default;

	virtual void loop() {}
};

#endif //PERSE_ROVER_ACTION_H