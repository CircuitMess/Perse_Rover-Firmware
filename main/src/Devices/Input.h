#ifndef CLOCKSTAR_FIRMWARE_INPUT_H
#define CLOCKSTAR_FIRMWARE_INPUT_H

#include "Util/Threaded.h"
#include <unordered_map>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <hal/gpio_types.h>

/**
 * The Curiosity board has no pairing button. Its only button is the power button, wired straight to
 * PIN_BTN through a divider off the battery, so it reads high while pressed.
 *
 * A short press behaves like the Perseverance pairing button press (it flips the camera in
 * DriveState), and holding it posts a Hold event that PowerButtonService turns into a power off.
 */
class Input : private Threaded {
public:
	Input();
	virtual ~Input();

	enum Button { Power };
	static const std::unordered_map<Button, const char*> PinLabels;

	struct Data {
		Button btn;
		enum Action { Release, Press, Hold } action;
	};

	bool getState(Button btn);

private:
	void scan();
	void pressed(Button btn);
	void released(Button btn);

	static const std::unordered_map<Button, uint8_t> PinMap;

	std::unordered_map<Button, bool> btnState;
	std::unordered_map<Button, uint64_t> pressTime;
	std::unordered_map<Button, bool> holdSent;

	std::unordered_map<Button, uint64_t> dbTime;
	static constexpr uint64_t SleepTime = 20; // [ms]
	static constexpr uint64_t DebounceTime = 30; // [ms]
	static constexpr uint64_t HoldTime = 1500; // [ms]

	void loop() override;

};


#endif //CLOCKSTAR_FIRMWARE_INPUT_H
