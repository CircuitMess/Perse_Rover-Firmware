#include "LEDModule.h"
#include "Util/stdafx.h"

LEDModule::LEDModule(ModuleBus bus) : Threaded("LEDModule", 2 * 1024), pinout(bus == ModuleBus::Left ? A_CTRL_1 : B_CTRL_1, false), queue(10){
	Events::listen(Facility::Comm, &queue);
	Events::listen(Facility::TCP, &queue);
	start();
}

LEDModule::~LEDModule(){
	stop(0);
	queue.unblock();
	while(running()){
		delayMillis(1);
	}
	Events::unlisten(&queue);
}

void LEDModule::loop(){
	Event event;
	if(!queue.get(event, portMAX_DELAY)) return;

	if(event.facility == Facility::Comm){
		Comm::Event& data = *((Comm::Event*) event.data);
		if(data.type == CommType::Headlights){
			pinout.set(data.headlights == HeadlightsMode::On);
		}
	}else if(event.facility == Facility::TCP){
		TCPServer::Event& data = *((TCPServer::Event*) event.data);
		if(data.status == TCPServer::Event::Status::Disconnected){
			pinout.off();
		}
	}

	free(event.data);
}
