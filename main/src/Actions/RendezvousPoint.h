#ifndef PERSE_ROVER_RENDEZVOUSPOINT_H
#define PERSE_ROVER_RENDEZVOUSPOINT_H

#include "PlayAudioAction.h"

class RendezvousPointAction : public PlayAudioAction {
	inline virtual constexpr const char* getFile() const override{ return "/spiffs/Markers/Rende.aac"; }
};

#endif //PERSE_ROVER_RENDEZVOUSPOINT_H
