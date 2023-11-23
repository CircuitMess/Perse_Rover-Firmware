#ifndef PERSE_ROVER_LIFEDETECTEDACTION_H
#define PERSE_ROVER_LIFEDETECTEDACTION_H

#include "PlayAudioAction.h"

class LifeDetectedAction : public PlayAudioAction {
	inline virtual constexpr char* getFile() const override{ return ""; } // TODO
};

#endif //PERSE_ROVER_LIFEDETECTEDACTION_H
