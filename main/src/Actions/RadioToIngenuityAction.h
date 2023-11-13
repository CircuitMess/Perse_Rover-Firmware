#ifndef PERSE_ROVER_RADIOTOINGENUITYACTION_H
#define PERSE_ROVER_RADIOTOINGENUITYACTION_H

#include "PlayAudioAction.h"

class RadioToIngenuityAction : public PlayAudioAction {
	inline virtual constexpr char* getFile() const override{ return ""; } // TODO
};

#endif //PERSE_ROVER_RADIOTOINGENUITYACTION_H