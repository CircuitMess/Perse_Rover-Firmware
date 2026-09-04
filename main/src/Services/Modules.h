#ifndef BIT_FIRMWARE_ROBOTS_H
#define BIT_FIRMWARE_ROBOTS_H

#include <map>
#include "Util/Threaded.h"
#include "Pins.hpp"
#include "Devices/TCA9555.h"
#include "CommData.h"
#include "Comm.h"
#include "Periph/ADC.h"
#include "Audio.h"

/**
 * The Curiosity board has a single module (UMAX) port, on the left side of the rover, so only
 * ModuleBus::Left is ever reported. Unlike Perseverance, the port has its own I2C bus - i2cUmax -
 * while the detect/address pins still hang off the XL9555 on the main bus. The XL9555 is shared with
 * Battery, so it is passed in rather than constructed here.
 */
class Modules : private SleepyThreaded {
public:
	Modules(TCA9555& tca, I2C& i2cUmax, ADC& adc);
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

	/** The only module port this board has. */
	static constexpr ModuleBus Bus = ModuleBus::Left;

private:
	I2C& i2cUmax;
	Comm& comm;
	ADC& adc;
	Audio* audio = nullptr;

	TCA9555& tca;
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

	BusContext context = { { TCA_ADDR_1, TCA_ADDR_2, TCA_ADDR_3, TCA_ADDR_4, TCA_ADDR_5, TCA_ADDR_6 },
						   { TCA_DET_1, TCA_DET_2 },
						   false, ModuleType::Unknown, nullptr };

	void sleepyLoop() override;

	bool checkInserted();
	ModuleType checkAddr();

	void loopCheck();

	static const std::unordered_map<uint8_t, ModuleType> AddressMap;
	static const std::unordered_map<uint8_t, ModuleType> I2CAddressMap;
	static constexpr uint8_t I2CModuleAddress = 63;
	struct ModuleAudio{
		const char* insertedPath;
		const char* removedPath;
	};
	static const std::unordered_map<ModuleType, ModuleAudio> AudioFilesMap;

};


#endif //BIT_FIRMWARE_ROBOTS_H
