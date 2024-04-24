#ifndef PERSE_ROVER_CAMERAPROSPECTAROUNDACTION_H
#define PERSE_ROVER_CAMERAPROSPECTAROUNDACTION_H

#include <cstdint>
#include "PlayAudioAction.h"

class CameraProspectAroundAction : public PlayAudioAction{
public:
	CameraProspectAroundAction();
	virtual ~CameraProspectAroundAction() override;

protected:
	inline virtual constexpr const char* getFile() const override {
		return "/spiffs/Markers/Scouting.aac";
	}

	virtual void loop() override;
	virtual bool readyToTransition() const override;

private:
	static constexpr uint64_t RotateDelay = 1000;
	uint64_t startTime = 0;
	class CameraController* controller = nullptr;
};

#endif //PERSE_ROVER_CAMERAPROSPECTAROUNDACTION_H