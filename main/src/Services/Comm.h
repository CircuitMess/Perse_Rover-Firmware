#ifndef PERSE_ROVER_COMM_H
#define PERSE_ROVER_COMM_H

#include "TCPServer.h"
#include "Util/Threaded.h"
#include "Util/Events.h"
#include <CommData.h>

class Comm : private Threaded {
public:
	struct Event {
		CommType type;
		union {
			DriveDir dir;
			HeadlightsMode headlights;
		};
		uint8_t raw;
	};

	Comm();
	~Comm() override;

	void sendHeadlightsState(HeadlightsMode headlights);

private:
	TCPServer& tcp;
	void loop() override;
	void sendPacket(const ControlPacket& packet);
	Event processPacket(const ControlPacket& packet);

	EventQueue queue;
};


#endif //PERSE_ROVER_COMM_H
