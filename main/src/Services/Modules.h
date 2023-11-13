#ifndef BIT_FIRMWARE_ROBOTS_H
#define BIT_FIRMWARE_ROBOTS_H

#include <map>
#include "Util/Threaded.h"
#include "Pins.hpp"
#include "Devices/TCA9555.h"
#include "CommData.h"
#include "Comm.h"
#include "Periph/ADC.h"


class Modules : private SleepyThreaded {
public:
	Modules(I2C& i2c, ADC& adc);
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
	I2C& i2c;
	Comm& comm;
	ADC& adc;

	TCA9555 tca;
	ThreadedClosure connectionThread;
	EventQueue connectionQueue;
	bool modulesEnabled = false;
	void connectionLoop();

	static constexpr uint32_t CheckInterval = 500; // [ms]

	struct BusContext {
		const uint8_t AddrPins[6];
		const uint8_t DetPins[2];

		bool inserted;
		ModuleType current;
		void* moduleInstance;
	};

	BusContext leftContext = { { TCA_A_ADDR_1, TCA_A_ADDR_2, TCA_A_ADDR_3, TCA_A_ADDR_4, TCA_A_ADDR_5, TCA_A_ADDR_6 },
							   { TCA_A_DET_1, TCA_A_DET_2 },
							   false, ModuleType::Unknown, nullptr };
	BusContext rightContext = { { TCA_B_ADDR_1, TCA_B_ADDR_2, TCA_B_ADDR_3, TCA_B_ADDR_4, TCA_B_ADDR_5, TCA_B_ADDR_6 },
								{ TCA_B_DET_1, TCA_B_DET_2 },
								false, ModuleType::Unknown, nullptr };

	void sleepyLoop() override;

	bool checkInserted(ModuleBus bus);
	ModuleType checkAddr(ModuleBus bus);

	BusContext& getContext(ModuleBus bus);

	void loopCheck(ModuleBus bus);

	static const std::unordered_map<uint8_t, ModuleType> AddressMap;
	static const std::unordered_map<uint8_t, ModuleType> I2CAddressMap;
	static constexpr uint8_t I2CModuleAddress = 63;

//	static constexpr uint8_t TesterBobAddr = Module::Bob | 0b00100000;

};


#endif //BIT_FIRMWARE_ROBOTS_H
