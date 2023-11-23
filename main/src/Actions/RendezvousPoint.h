#ifndef PERSE_ROVER_RENDEZVOUSPOINT_H
#define PERSE_ROVER_RENDEZVOUSPOINT_H

#include "PlayAudioAction.h"

class RendezvousPointAction : public PlayAudioAction {
	inline virtual constexpr char* getFile() const override{ return ""; } // TODO
};

#endif //PERSE_ROVER_RENDEZVOUSPOINT_H
