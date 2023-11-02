#include "HeadlightsController.h"
#include "Services/Comm.h"
#include "Util/Services.h"
#include "Services/LEDService.h"

HeadlightsController::HeadlightsController() : DeviceController("Headlights Controller"){
	setControl(DeviceControlType::Local);
	setLocally(HeadlightsState{});
	setControl(DeviceControlType::Remote);
}

void HeadlightsController::write(const HeadlightsState& state){
	LEDService* ledService = (LEDService*)Services.get(Service::LED);
	if (ledService == nullptr) {
		return;
	}

	if (state.Mode == HeadlightsMode::Off){
		ledService->off(LED::LeftHeadlight);
		ledService->off(LED::RightHeadlight);
	}
	else{
		ledService->on(LED::LeftHeadlight);
		ledService->on(LED::RightHeadlight);
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