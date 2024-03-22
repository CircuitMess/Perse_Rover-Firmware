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

	for(int i = 1; i <= RandSamplesNum; ++i){
		randIdSet.insert(i);
	}
}

void RandSoundPlayer::loop(){
	if(millis() - counter <= randThreshold) return;

	counter = millis();
	randThreshold = getRandThresh();
	ESP_LOGD(TAG, "randThresh: %lu\n", randThreshold);

	if(randIdSet.empty()){
		for(int i = 1; i <= RandSamplesNum; ++i){
			randIdSet.insert(i);
		}
	}
	int randSetElement = rand() % randIdSet.size();
	auto it = randIdSet.begin();
	std::advance(it, randSetElement);
	uint8_t randId = *it;
	randIdSet.erase(it);

	std::string randPath = "/spiffs/EasterEggs/Random" + std::to_string(randId) + ".aac";

	audio.play(randPath);
}

void RandSoundPlayer::resetTimer(){
	counter = millis();
}

uint32_t RandSoundPlayer::getRandThresh(){
	return RandThreshMin + (rand() % (RandThreshMax - RandThreshMin + 1));
}
