#ifndef PERSE_ROVER_ALERTACTION_H
#define PERSE_ROVER_ALERTACTION_H

#include "PlayAudioAction.h"

class AlertAction : public PlayAudioAction {
	inline virtual constexpr const char* getFile() const override{ return "/spiffs/Markers/Alert.aac"; }
};

#endif //PERSE_ROVER_ALERTACTION_H