#ifndef PERSE_ROVER_CAMERACONTROLLER_H
#define PERSE_ROVER_CAMERACONTROLLER_H

#include "DeviceController.h"
#include "CommData.h"
#include <glm.hpp>

struct CameraState
{
	CameraRotation Rotation = 35;
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
	virtual void processEvent(const Event& event) override;

private:
	class Servo* cameraServo = nullptr;
	static constexpr glm::vec<2, uint8_t> rotationLimits = {10, 100};
};

#endif //PERSE_ROVER_CAMERACONTROLLER_H