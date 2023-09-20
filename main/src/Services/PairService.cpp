#include "PairService.h"
#include "Util/Services.h"
#include "Util/Events.h"
#include "Util/stdafx.h"

PairService::PairService() : Threaded("PairService", 4 * 1024),
							 wifi(*(WiFiAP*) Services.get(Service::WiFi)),
							 tcp(*(TCPServer*) Services.get(Service::TCP)){
	wifi.setHidden(false);
	start();
}

PairService::~PairService(){
	wifi.setHidden(true);
	stop();
}

void PairService::loop(){
	bool accepted = tcp.accept();
	if(accepted){
		state = State::Success;
		Event evt{ true };
		Events::post(Facility::Pair, evt);
		stop();
		return;
	}
	delayMillis(100);
}

PairService::State PairService::getState() const{
	return state;
}
