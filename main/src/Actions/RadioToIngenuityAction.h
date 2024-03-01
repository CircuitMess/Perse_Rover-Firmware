#ifndef PERSE_ROVER_RADIOTOINGENUITYACTION_H
#define PERSE_ROVER_RADIOTOINGENUITYACTION_H

#include "PlayAudioAction.h"

class RadioToIngenuityAction : public PlayAudioAction {
	inline virtual constexpr const char* getFile() const override{ return "/spiffs/Markers/Ingenuity.aac"; }
};

#endif //PERSE_ROVER_RADIOTOINGENUITYACTION_H