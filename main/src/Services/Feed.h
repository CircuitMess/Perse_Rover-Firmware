#ifndef PERSE_ROVER_FEED_H
#define PERSE_ROVER_FEED_H

#include "UDPEmitter.h"
#include <DriveInfo.h>

class Feed {
public:
	Feed();
	virtual ~Feed();

	void sendFrame(const DriveInfo& frame);

private:
	UDPEmitter udp;

	static constexpr size_t TxBufSize = 10000;
	uint8_t* txBuf;
};


#endif //PERSE_ROVER_FEED_H
