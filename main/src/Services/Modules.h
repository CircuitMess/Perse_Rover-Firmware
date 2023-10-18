#ifndef BIT_FIRMWARE_ROBOTS_H
#define BIT_FIRMWARE_ROBOTS_H

#include <map>
#include "Util/Threaded.h"
#include "Pins.hpp"
#include "Devices/ShiftReg.h"
#include "CommData.h"
#include "Comm.h"


class Modules : private SleepyThreaded {
public:
	Modules(ShiftReg& shiftReg, I2C& i2c, Comm& comm);
	~Modules() override;

	struct Event {
		enum {
			Insert, Remove
		} action;
		ModuleBus bus;
		ModuleType module;
	};

	ModuleType getInserted(ModuleBus bus);

	static constexpr TickType_t ModuleSendInterval = 200;

private:
	ShiftReg& shiftReg;
	I2C& i2c;
	Comm& comm;

	static constexpr uint32_t CheckInterval = 500; // [ms]

	struct BusContext {
		const uint8_t AddrPins[6];
		const uint8_t DetPins[2];

		bool inserted;
		ModuleType current;
		void* moduleInstance;
	};

	BusContext leftContext = { { SHIFT_A_ADDR_1, SHIFT_A_ADDR_2, SHIFT_A_ADDR_3, SHIFT_A_ADDR_4, SHIFT_A_ADDR_5, SHIFT_A_ADDR_6 },
							   { SHIFT_A_DET_1, SHIFT_A_DET_2 },
							   false, ModuleType::Unknown, nullptr };
	BusContext rightContext = { { SHIFT_B_ADDR_1, SHIFT_B_ADDR_2, SHIFT_B_ADDR_3, SHIFT_B_ADDR_4, SHIFT_B_ADDR_5, SHIFT_B_ADDR_6 },
								{ SHIFT_B_DET_1, SHIFT_B_DET_2 },
								false, ModuleType::Unknown, nullptr };

	void sleepyLoop() override;

	bool checkInserted(ModuleBus bus);
	ModuleType checkAddr(ModuleBus bus);

	BusContext& getContext(ModuleBus bus);

	void loopCheck(ModuleBus bus);

	static const std::map<uint8_t, ModuleType> AddressMap;
	static const std::map<uint8_t, ModuleType> I2CAddressMap;
	static constexpr uint8_t I2CModuleAddress = 63;

//	static constexpr uint8_t TesterBobAddr = Module::Bob | 0b00100000;

};


#endif //BIT_FIRMWARE_ROBOTS_H
