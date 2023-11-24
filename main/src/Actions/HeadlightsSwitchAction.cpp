#include "HeadlightsSwitchAction.h"
#include "Util/Services.h"
#include "Devices/HeadlightsController.h"

HeadlightsSwitchAction::HeadlightsSwitchAction(){
	HeadlightsController* controller = (HeadlightsController*) Services.get(Service::HeadLightsController);

	if(controller == nullptr){
		return;
	}

	controller->setControl(DeviceControlType::Local);

	HeadlightsState state {.Mode = HeadlightsMode::Off};

	if (controller->getCurrentState().Mode == HeadlightsMode::Off) {
		state.Mode = HeadlightsMode::On;
	}

	controller->setLocally(state);
	controller->setControl(DeviceControlType::Remote);

	markForDestroy();
}