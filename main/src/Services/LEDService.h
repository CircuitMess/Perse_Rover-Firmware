#ifndef PERSE_ROVER_LEDSERVICE_H
#define PERSE_ROVER_LEDSERVICE_H

#include <cstdint>
#include <map>
#include <tuple>
#include <driver/ledc.h>
#include <memory>
#include <mutex>
#include "Devices/AW9523.h"
#include "Util/Threaded.h"
#include "Util/Queue.h"

enum class LED : uint8_t {
	Camera,
	Rear,
	MotorLeft,
	MotorRight,
	Arm,
	HeadlightLeft,
	HeadlightsRight,
	StatusGreen,
	StatusYellow,
	StatusRed,
	COUNT
};

class LEDService : private Threaded {
public:
	explicit LEDService(AW9523& aw9523);

	virtual ~LEDService();

	void on(LED led);

	void off(LED led);

	void blink(LED led, uint32_t count = 1, uint32_t period = 1000);

	void breathe(LED led, uint32_t period = 1000);

	void set(LED led, float percent);

	void breatheTo(LED led, float targetPercent, uint32_t duration = 250);

protected:
	virtual void loop() override;

private:
	struct PwnMappingInfo {
		gpio_num_t pin = GPIO_NUM_NC;
		ledc_channel_t channel = LEDC_CHANNEL_0;
		uint8_t limit = 100;
	};

	struct ExpanderMappingInfo {
		uint8_t pin = 0;
		uint8_t limit = 0xFF;
	};

	static const std::map<LED, PwnMappingInfo> PwmMappings;
	static const std::map<LED, ExpanderMappingInfo> ExpanderMappings;

private:
	enum LEDInstruction {
		On,
		Off,
		Blink,
		Breathe,
		Set,
		BreatheTo
	};

	struct LEDInstructionInfo {
		LED led;
		LEDInstruction instruction;
		uint32_t count;
		uint32_t period;
		float targetPercent;
	};

	std::map<LED, class SingleLED*> ledDevices;
	std::map<LED, std::unique_ptr<class LEDFunction>> ledFunctions;
	Queue<LEDInstructionInfo> instructionQueue;

private:
	void onInternal(LED led);

	void offInternal(LED led);

	void blinkInternal(LED led, uint32_t count, uint32_t period);

	void breatheInternal(LED led, uint32_t period);

	void setInternal(LED led, float percent);

	void breatheToInternal(LED led, float targetPercent, uint32_t duration);
};

#endif //PERSE_ROVER_LEDSERVICE_H