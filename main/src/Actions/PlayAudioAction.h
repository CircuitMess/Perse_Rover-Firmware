#ifndef PERSE_ROVER_PLAYAUDIOACTION_H
#define PERSE_ROVER_PLAYAUDIOACTION_H

#include "Action.h"

class PlayAudioAction : public Action{
public:
	inline virtual constexpr const char* getFile() const = 0;

protected:
	virtual void loop() override;

private:
	bool played = false;
};

#endif //PERSE_ROVER_PLAYAUDIOACTION_H