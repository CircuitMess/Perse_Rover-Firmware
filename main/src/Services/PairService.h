#ifndef PERSE_ROVER_PAIRSERVICE_H
#define PERSE_ROVER_PAIRSERVICE_H

#include "Periph/WiFiAP.h"
#include "TCPServer.h"
#include "Util/Threaded.h"

class PairService : private Threaded {
public:
	PairService();
	~PairService() override;

	struct Event {
		bool success;
	};
private:
	WiFiAP& wifi;
	TCPServer& tcp;

	void loop() override;
};


#endif //PERSE_ROVER_PAIRSERVICE_H
