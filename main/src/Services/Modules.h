#ifndef BIT_FIRMWARE_ROBOTS_H
#define BIT_FIRMWARE_ROBOTS_H

#include "Util/Threaded.h"
#include "Pins.hpp"
#include "Devices/ShiftReg.h"

enum class Module : uint8_t {
	Gyro, RGB, AltPress, TempHum, PhotoRes, LED, CO2, Motion, COUNT
};
enum class ModuleBus : uint8_t {
	Bus_A = 0, Bus_B = 1
};

class Modules : private SleepyThreaded {
public:
	Modules(ShiftReg& shiftReg);
	~Modules() override;

	struct Event {
		enum {
			Insert, Remove
		} action;
		ModuleBus bus;
		Module module;
	};

	Module getInserted(ModuleBus bus);

private:
	ShiftReg& shiftReg;

	static constexpr uint32_t CheckInterval = 500; // [ms]

	struct BusContext {
		const uint8_t AddrPins[6];
		const uint8_t DetPins[2];

		bool inserted;
		uint8_t current;
	};

	BusContext A_context = { { SHIFT_A_ADDR_1, SHIFT_A_ADDR_2, SHIFT_A_ADDR_3, SHIFT_A_ADDR_4, SHIFT_A_ADDR_5, SHIFT_A_ADDR_6 },
							 { SHIFT_A_DET_1, SHIFT_A_DET_2 },
							 false, 255 };
	BusContext B_context = { { SHIFT_B_ADDR_1, SHIFT_B_ADDR_2, SHIFT_B_ADDR_3, SHIFT_B_ADDR_4, SHIFT_B_ADDR_5, SHIFT_B_ADDR_6 },
							 { SHIFT_B_DET_1, SHIFT_B_DET_2 },
							 false, 255 };

	void sleepyLoop() override;

	bool checkInserted(ModuleBus bus);
	uint8_t checkAddr(ModuleBus bus);

	BusContext& getContext(ModuleBus bus);

	void loopCheck(ModuleBus bus);

//	static constexpr uint8_t TesterBobAddr = Module::Bob | 0b00100000;

};


#endif //BIT_FIRMWARE_ROBOTS_H
