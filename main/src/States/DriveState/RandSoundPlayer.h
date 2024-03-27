#ifndef PERSE_ROVER_RANDSOUNDPLAYER_H
#define PERSE_ROVER_RANDSOUNDPLAYER_H

#include <unordered_set>
#include "Services/Audio.h"

class RandSoundPlayer {
public:
	RandSoundPlayer();
	void loop();
	void resetTimer();

private:
	static inline uint32_t getRandThresh();

	Audio& audio;
	uint32_t randThreshold = 0;
	uint32_t counter = 0; //TODO - implement using HW timers

	//10-20s between random sounds
	static constexpr uint32_t RandThreshMin = 20000; //[ms]
	static constexpr uint32_t RandThreshMax = 40000; //[ms]

	static constexpr uint32_t RandSamplesNum = 5;
	std::unordered_set<uint8_t> randIdSet;
};


#endif //PERSE_ROVER_RANDSOUNDPLAYER_H
