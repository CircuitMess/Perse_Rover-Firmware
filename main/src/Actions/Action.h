#ifndef PERSE_ROVER_ACTION_H
#define PERSE_ROVER_ACTION_H

class Action {
public:
	Action() = default;
	virtual ~Action() = default;

	virtual void loop() {}
	virtual bool readyToTransition() const { return true; }

	inline void markForDestroy() { markedForDestroy = true; }
	inline bool isMarkedForDestroy() const { return markedForDestroy; }

private:
	bool markedForDestroy = false;
};

#endif //PERSE_ROVER_ACTION_H