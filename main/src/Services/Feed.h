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

	void disableScanning();

	void flipCam(bool flip);

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
	MarkerAction oldAction = MarkerAction::None;
	bool shouldPlayAudioOnCamFailure = true;

	struct EventData {
		enum Type {
			None,
			ScanningEnableChange,
			FeedQualityChange,
			CamFlip
		};

		Type type = None;
		union {
			bool isScanningEnabled = false;
			uint8_t feedQuality;
			bool flip;
		};
	};
	Queue<EventData> communicationQueue;

	static constexpr glm::vec<2, uint8_t> QualityLimits = { 0, 30};
	static constexpr size_t TxBufSize = 10000;
	uint8_t* txBuf;

	uint8_t frameFilterCounter = 0;
	static constexpr uint8_t FrameFilterCount = 4; //scanned marker always persists for at least 4 frames, to smoothen recognition

private:
	void sendFrame();
};

#endif //PERSE_ROVER_FEED_H