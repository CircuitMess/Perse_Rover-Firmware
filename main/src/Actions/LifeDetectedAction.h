#ifndef PERSE_ROVER_LIFEDETECTEDACTION_H
#define PERSE_ROVER_LIFEDETECTEDACTION_H

#include "PlayAudioAction.h"

class LifeDetectedAction : public PlayAudioAction {
	inline virtual constexpr const char* getFile() const override{ return "/spiffs/Markers/Life.aac"; }
};

#endif //PERSE_ROVER_LIFEDETECTEDACTION_H
