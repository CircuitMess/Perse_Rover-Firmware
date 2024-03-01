#ifndef PERSE_ROVER_CALLSAMPLELANDER_H
#define PERSE_ROVER_CALLSAMPLELANDER_H

#include "PlayAudioAction.h"

class CallSampleLanderAction : public PlayAudioAction {
	inline virtual constexpr const char* getFile() const override{ return "/spiffs/Markers/Samples.aac"; }
};

#endif //PERSE_ROVER_CALLSAMPLELANDER_H
