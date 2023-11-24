#ifndef PERSE_ROVER_ALERTACTION_H
#define PERSE_ROVER_ALERTACTION_H

#include "PlayAudioAction.h"

class AlertAction : public PlayAudioAction {
	inline virtual constexpr char* getFile() const override{ return ""; } // TODO
};

#endif //PERSE_ROVER_ALERTACTION_H