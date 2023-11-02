#ifndef CLOCKSTAR_FIRMWARE_INPUT_H
#define CLOCKSTAR_FIRMWARE_INPUT_H

#include "Util/Threaded.h"
#include "AW9523.h"
#include <unordered_map>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <hal/gpio_types.h>

class Input : private Threaded {
public:
	Input(AW9523& aw9523);
	virtual ~Input();

	enum Button { Pair };
	static const std::unordered_map<Button, const char*> PinLabels;

	struct Data {
		Button btn;
		enum Action { Release, Press } action;
	};

	bool getState(Button btn);

private:
	void scan();
	void pressed(Button btn);
	void released(Button btn);

	static const std::unordered_map<Button, uint8_t> PinMap;

	std::unordered_map<Button, bool> btnState;

	enum DbDir { Release, Press, None };
	std::unordered_map<Button, uint64_t> dbTime;
	static constexpr uint64_t SleepTime = 20; // [ms]
	static constexpr uint64_t DebounceTime = 5; // [ms]

	void loop() override;

	AW9523& aw9523;

};


#endif //CLOCKSTAR_FIRMWARE_INPUT_H
