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

enum class LED : uint8_t
{
	Camera,
	Rear,
	LeftMotor,
	RightMotor,
	Arm,
	LeftHeadlight,
	RightHeadlight,
	StatusGreen,
	StatusYellow,
	StatusRed,
	COUNT
};

class LEDService : private SleepyThreaded
{
public:
	explicit LEDService(AW9523& aw9523);
	virtual ~LEDService();

	void on(LED led);
	void off(LED led);
	void blink(LED led, uint32_t count = 1, uint32_t period = 500);
	void breathe(LED led, uint32_t period = 500);

protected:
	virtual void sleepyLoop() override;

private:
	static const std::map<LED, std::tuple<gpio_num_t, ledc_channel_t, uint8_t>> pwmMappings;
	static const std::map<LED, std::tuple<uint8_t, uint8_t>> expanderMappings;

private:
	std::map<LED, class SingleLED*> ledDevices;
	std::map<LED, std::unique_ptr<class LEDFunction>> ledFunctions;
	std::mutex mutex;
};

#endif //PERSE_ROVER_LEDSERVICE_H