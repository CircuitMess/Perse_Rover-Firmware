#include "HeadlightsSwitchAction.h"
#include "Util/Services.h"
#include "Devices/HeadlightsController.h"
#include "Services/Audio.h"

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

	if(Audio* audio = (Audio*) Services.get(Service::Audio)){
		if(state.Mode == HeadlightsMode::On){
			audio->play("/spiffs/Systems/LightOn.aac");
		}else if(state.Mode == HeadlightsMode::Off){
			audio->play("/spiffs/Systems/LightOff.aac");
		}
	}

	controller->setLocally(state);
	controller->setControl(DeviceControlType::Remote);

	markForDestroy();
}