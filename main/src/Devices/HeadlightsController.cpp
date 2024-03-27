#include "HeadlightsController.h"
#include "Services/Comm.h"
#include "Util/Services.h"
#include "Services/LEDService.h"
#include "Services/Audio.h"

HeadlightsController::HeadlightsController() : DeviceController("Headlights Controller", false){
	setControl(DeviceControlType::Local);
	setLocally(HeadlightsState{});
	setControl(DeviceControlType::Remote);
}

void HeadlightsController::write(const HeadlightsState& state){
	LEDService* ledService = (LEDService*) Services.get(Service::LED);
	if(ledService == nullptr){
		return;
	}

	if(state.Mode == HeadlightsMode::Off){
		ledService->off(LED::HeadlightLeft);
		ledService->off(LED::HeadlightsRight);
	}else{
		ledService->on(LED::HeadlightLeft);
		ledService->on(LED::HeadlightsRight);
	}
}

HeadlightsState HeadlightsController::getDefaultState() const{
	return HeadlightsState{};
}

void HeadlightsController::sendState(const HeadlightsState& state, bool local) const{
	auto comm = (Comm*) Services.get(Service::Comm);
	if(comm == nullptr){
		return;
	}

	comm->sendHeadlightsState(state.Mode, local);
}

void HeadlightsController::processEvent(const Event& event){
	auto* commEvent = (Comm::Event*) event.data;
	if(commEvent == nullptr){
		return;
	}

	if(commEvent->type != CommType::Headlights){
		return;
	}

	HeadlightsState state = {
			.Mode = commEvent->headlights
	};

	Audio* audio = (Audio*) Services.get(Service::Audio);

	if(getCurrentState().Mode != state.Mode){
		if(commEvent->headlights == HeadlightsMode::On){
			audio->play("/spiffs/Systems/LightOn.aac");
		}else if(commEvent->headlights == HeadlightsMode::Off){
			audio->play("/spiffs/Systems/LightOff.aac");
		}
	}

	setRemotely(state);
}