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

	enum class State{
		Pairing, Success
	};
	State getState() const;

private:
	WiFiAP& wifi;
	TCPServer& tcp;

	State state = State::Pairing;

	void loop() override;
};


#endif //PERSE_ROVER_PAIRSERVICE_H
