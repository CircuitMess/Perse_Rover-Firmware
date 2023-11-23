#include "PlayAudioAction.h"
#include "Services/Audio.h"
#include "Util/Services.h"

void PlayAudioAction::loop(){
	if (played){
		return;
	}

	Audio* audio = (Audio*) Services.get(Service::Audio);
	if (audio == nullptr) {
		return;
	}

	audio->stop();
	audio->play(getFile());

	played = true;
}
