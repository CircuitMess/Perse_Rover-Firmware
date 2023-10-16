#ifndef PERSE_ROVER_CAMERACONTROLLER_H
#define PERSE_ROVER_CAMERACONTROLLER_H

#include "DeviceController.h"
#include "CommData.h"

struct CameraState
{
	CameraRotation Rotation = 100;
};

class CameraController : public DeviceController<CameraState>
{
public:
	CameraController();
	virtual ~CameraController() override;

protected:
	virtual void write(const CameraState& state) override;
	virtual CameraState getDefaultState() const override;
	virtual void sendState(const CameraState& state) const override;
	virtual CameraState processStateFromEvent(const Event& event) const override;

private:
	class Servo* cameraServo = nullptr;
};

#endif //PERSE_ROVER_CAMERACONTROLLER_H