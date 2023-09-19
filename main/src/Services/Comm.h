#ifndef PERSE_ROVER_COMM_H
#define PERSE_ROVER_COMM_H

#include "TCPServer.h"
#include "Util/Threaded.h"
#include "Util/Events.h"
#include <Comm.h>

class Comm : private Threaded {
public:
	Comm();
	~Comm() override;
	void sendPacket(const ControlPacket& packet);

private:
	TCPServer& tcp;
	void loop() override;

	EventQueue queue;
};


#endif //PERSE_ROVER_COMM_H
