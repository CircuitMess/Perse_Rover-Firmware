#ifndef PERSE_MISSIONCTRL_LED_H
#define PERSE_MISSIONCTRL_LED_H

#include "Devices/AW9523.h"
#include "Util/Queue.h"
#include "Util/Threaded.h"

class LED : private Threaded {
public:
	explicit LED(AW9523& aw9523);

	void on(uint8_t i);
	void off(uint8_t i);
	void blink(uint8_t i);
	void blinkCont(uint8_t i);

private:
	AW9523& expander;

	enum LEDAction { Off, On, Blink, BlinkCont };
	std::array<LEDAction, 16> actions;
	std::array<bool, 16> state;
	std::array<uint64_t, 16> times;

	struct Action {
		uint8_t pin;
		LEDAction action;
	};

	Queue<Action> actionQueue;

	void loop() override;

};


#endif //PERSE_MISSIONCTRL_LED_H
