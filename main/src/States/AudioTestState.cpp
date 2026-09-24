#include "AudioTestState.h"
#include <esp_log.h>
#include "Devices/Input.h"
#include "Services/Audio.h"
#include "Util/Services.h"

static const char* TAG = "AudioTestState";

AudioTestState::AudioTestState() : evts(6){
	Events::listen(Facility::Input, &evts);

	ESP_LOGI(TAG, "Curiosity audio sample test, %u samples:", (unsigned) SampleCount);
	for(size_t i = 0; i < SampleCount; i++){
		ESP_LOGI(TAG, "  %2u. %s", (unsigned) (i + 1), Samples[i]);
	}
	ESP_LOGI(TAG, "Press the power button to play the next sample. Hold it for 1.5 s to power off.");

	// Speaker check, so a silent board is told apart from a silent sample.
	if(Audio* audio = (Audio*) Services.get(Service::Audio)){
		audio->play("/spiffs/Beep.aac", true);
	}
}

AudioTestState::~AudioTestState(){
	Events::unlisten(&evts);
}

void AudioTestState::unblock(){
	evts.unblock();
}

void AudioTestState::loop(){
	Event evt{};
	if(!evts.get(evt, portMAX_DELAY)) return;

	if(evt.facility == Facility::Input && evt.data != nullptr){
		const auto* data = (Input::Data*) evt.data;
		if(data->btn == Input::Power && data->action == Input::Data::Press){
			playNext();
		}
	}

	free(evt.data);
}

void AudioTestState::playNext(){
	index = (index + 1) % SampleCount;

	ESP_LOGI(TAG, "[%u/%u] %s", (unsigned) (index + 1), (unsigned) SampleCount, Samples[index]);

	if(Audio* audio = (Audio*) Services.get(Service::Audio)){
		audio->play(Samples[index], true, true);
	}else{
		ESP_LOGE(TAG, "Audio service missing");
	}
}
