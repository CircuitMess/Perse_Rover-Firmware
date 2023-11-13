#ifndef PERSE_ROVER_CALLSAMPLELANDER_H
#define PERSE_ROVER_CALLSAMPLELANDER_H

#include "PlayAudioAction.h"

class CallSampleLanderAction : public PlayAudioAction {
	inline virtual constexpr char* getFile() const override{ return ""; } // TODO
};

#endif //PERSE_ROVER_CALLSAMPLELANDER_H
