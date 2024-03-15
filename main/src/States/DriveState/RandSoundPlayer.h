#ifndef PERSE_ROVER_RANDSOUNDPLAYER_H
#define PERSE_ROVER_RANDSOUNDPLAYER_H

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
	static constexpr uint32_t RandThreshMin = 10000; //[ms]
	static constexpr uint32_t RandThreshMax = 20000; //[ms]
};


#endif //PERSE_ROVER_RANDSOUNDPLAYER_H
