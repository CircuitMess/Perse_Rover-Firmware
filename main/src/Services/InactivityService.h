#ifndef PERSE_ROVER_INACTIVITYSERVICE_H
#define PERSE_ROVER_INACTIVITYSERVICE_H

#include <atomic>
#include "Util/Threaded.h"
#include "Util/Events.h"

class InactivityService : public Threaded {
public:
	InactivityService();

protected:
	void loop() override;

private:
	bool checkActions();

	uint32_t timer = 0;
	EventQueue queue;
	bool paired = false;

	static constexpr uint32_t UnpairedTimeout = 300000; //5 mins
	static constexpr uint32_t PairedTimeout = 600000; //10 mins
};


#endif //PERSE_ROVER_INACTIVITYSERVICE_H
