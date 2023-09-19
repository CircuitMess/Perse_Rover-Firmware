#include "PairService.h"
#include "Util/Services.h"
#include "Util/Events.h"

PairService::PairService() : Threaded("PairService", 4 * 1024),
							 wifi(*(WiFiAP*) Services.get(Service::WiFi)),
							 tcp(*(TCPServer*) Services.get(Service::TCP)){
	wifi.setHidden(false);
	start();
}

PairService::~PairService(){
	wifi.setHidden(true);
	if(running()){
		stop();
	}
}

void PairService::loop(){
	bool accepted = tcp.accept();
	if(accepted){
		Event evt{ true };
		Events::post(Facility::Pair, evt);
		stop();
		return;
	}
	vTaskDelay(100 / portTICK_PERIOD_MS);
}