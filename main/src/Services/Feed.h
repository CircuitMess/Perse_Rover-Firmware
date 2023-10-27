#ifndef PERSE_ROVER_FEED_H
#define PERSE_ROVER_FEED_H

#include <DriveInfo.h>
#include <atomic>
#include "Util/Threaded.h"
#include "UDPEmitter.h"
#include "Devices/Camera.h"
#include "Util/Events.h"

class Feed : private Threaded {
public:
	Feed(I2C& i2c);
	virtual ~Feed();

protected:
	virtual void loop() override;

private:
	UDPEmitter udp;
	EventQueue queue;
	std::atomic<uint8_t> feedQuality = 0; // [0 - 10], if 0, camera feed doesn't get sent
	SleepyThreadedClosure frameSendingThread;
	Camera* camera;

	static constexpr size_t TxBufSize = 10000;
	uint8_t* txBuf;

private:
	void sendFrame();
	void onSendingStarted();
	void onSendingEnded();
	void onFeedQualityChanged(uint8_t oldFeedQuality);
};

#endif //PERSE_ROVER_FEED_H