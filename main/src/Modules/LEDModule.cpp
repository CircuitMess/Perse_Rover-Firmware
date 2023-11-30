#include "LEDModule.h"
#include "Util/stdafx.h"
#include "Util/Services.h"
#include "Devices/HeadlightsController.h"

LEDModule::LEDModule(ModuleBus bus) : SleepyThreaded(10, "LEDModule", 2 * 1024), pinout(bus == ModuleBus::Left ? A_CTRL_1 : B_CTRL_1, true), queue(10), bus(bus){
	Events::listen(Facility::Comm, &queue);
	Events::listen(Facility::TCP, &queue);
	start();

	if(const HeadlightsController* controller = (HeadlightsController*) Services.get(Service::HeadLightsController)){
		state = controller->getCurrentState().Mode == HeadlightsMode::On;
		pinout.set(state);

		if(Comm* comm = (Comm*) Services.get(Service::Comm)){
			const ModuleData data = {
					ModuleType::LED,
					bus,
					{ .ledState = { state } }
			};

			comm->sendModuleData(data);
		}
	}
}

LEDModule::~LEDModule(){
	stop(0);
	queue.unblock();

	while(running()){
		delayMillis(1);
	}

	Events::unlisten(&queue);
}

void LEDModule::sleepyLoop(){
	const bool oldState = state;

	for(Event event = {}; queue.get(event, 0); ){
		if(event.facility == Facility::Comm){
			Comm::Event& data = *((Comm::Event*) event.data);
			if(data.type == CommType::Headlights){
				state = data.headlights == HeadlightsMode::On;
				pinout.set(state);
			}
		}else if(event.facility == Facility::TCP){
			TCPServer::Event& data = *((TCPServer::Event*) event.data);
			if(data.status == TCPServer::Event::Status::Disconnected){
				pinout.off();
				state = false;
			}
		}

		free(event.data);
	}

	if(state == oldState){
		return;
	}

	Comm* comm = (Comm*) Services.get(Service::Comm);
	if(comm == nullptr){
		return;
	}

	const ModuleData data = {
			ModuleType::LED,
			bus,
			{ .ledState = { state } }
	};

	comm->sendModuleData(data);
}
