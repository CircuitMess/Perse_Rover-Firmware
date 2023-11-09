#ifndef PERSE_ROVER_FEED_H
#define PERSE_ROVER_FEED_H

#include <DriveInfo.h>
#include <atomic>
#include <memory>
#include <glm.hpp>
#include "Util/Threaded.h"
#include "UDPEmitter.h"
#include "Devices/Camera.h"
#include "Util/Events.h"
#include "Util/MarkerScanner.h"
#include "Util/Queue.h"

class Feed : private SleepyThreaded {
public:
	enum class EventType {
		MarkerScanned
	};

	struct Event {
		EventType type;
		MarkerAction markerAction;
	};

	explicit Feed(I2C& i2c);

	virtual ~Feed();

protected:
	virtual void sleepyLoop() override;

private:
	UDPEmitter udp;
	EventQueue queue;
	std::atomic<uint8_t> feedQuality = 0; // [0 - 10], if 0, camera feed doesn't get sent
	std::atomic<bool> isScanningEnabled = false;
	SleepyThreadedClosure frameSendingThread;
	std::unique_ptr<Camera> camera;
	std::unique_ptr<MarkerScanner> markerScanner;

	struct EventData {
		enum Type {
			None,
			ScanningEnableChange,
			FeedQualityChange
		};

		Type type = None;
		union {
			bool isScanningEnabled = false;
			uint8_t feedQuality;
		};
	};
	Queue<EventData> communicationQueue;

	static constexpr glm::vec<2, uint8_t> qualityLimits = {1, 20};
	static constexpr size_t TxBufSize = 10000;
	uint8_t* txBuf;

private:
	void sendFrame();
};

#endif //PERSE_ROVER_FEED_H