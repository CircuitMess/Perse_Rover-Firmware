#include "RandSoundPlayer.h"
#include "Util/stdafx.h"
#include "Util/Services.h"
#include <esp_log.h>

static const char* TAG = "RandPlayer";

RandSoundPlayer::RandSoundPlayer() : audio(*(Audio*) Services.get(Service::Audio)){

	srand(millis() * millis());
	randThreshold = getRandThresh();
	ESP_LOGD(TAG, "randThresh: %lu\n", randThreshold);
	counter = millis();
}

void RandSoundPlayer::loop(){
	if(millis() - counter <= randThreshold) return;

	counter = millis();
	randThreshold = getRandThresh();
	ESP_LOGD(TAG, "randThresh: %lu\n", randThreshold);
	audio.play("/spiffs/audioOn.wav");//TODO - pick and play random sound
}

void RandSoundPlayer::resetTimer(){
	counter = millis();
}

uint32_t RandSoundPlayer::getRandThresh(){
	return RandThreshMin + (rand() % (RandThreshMax - RandThreshMin + 1));
}
