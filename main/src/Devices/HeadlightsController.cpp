#include "HeadlightsController.h"
#include "Pins.hpp"
#include "Services/Comm.h"
#include "Util/Services.h"

static const char* const TAG = "HeadlightsController";

HeadlightsController::HeadlightsController(AW9523& aw9523) : DeviceController("Headlights Controller"), aw9523(aw9523){
	aw9523.pinMode(EXP_HEADLIGHT_1, AW9523::LED);
	aw9523.pinMode(EXP_HEADLIGHT_2, AW9523::LED);
}

void HeadlightsController::write(const HeadlightsState& state){
	if (state.Mode == HeadlightsMode::Off){
		aw9523.dim(EXP_HEADLIGHT_1, 0);
		aw9523.dim(EXP_HEADLIGHT_2, 0);
	}
	else{
		aw9523.dim(EXP_HEADLIGHT_1, 255);
		aw9523.dim(EXP_HEADLIGHT_2, 255);
	}
}

HeadlightsState HeadlightsController::getDefaultState() const{
	return HeadlightsState{};
}

void HeadlightsController::sendState(const HeadlightsState& state) const{
	auto comm = (Comm*)Services.get(Service::Comm);
	if (comm == nullptr){
		return;
	}

	comm->sendHeadlightsState(state.Mode);
}

void HeadlightsController::processEvent(const Event& event) {
	auto* commEvent = (Comm::Event*)event.data;
	if (commEvent == nullptr){
		return;
	}

	if (commEvent->type != CommType::Headlights) {
		return;
	}

	HeadlightsState state = {
			.Mode = commEvent->headlights
	};

	setRemotely(state);
}