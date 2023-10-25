#ifndef PERSE_ROVER_MOTORDRIVECONTROLLER_H
#define PERSE_ROVER_MOTORDRIVECONTROLLER_H

#include "DeviceController.h"
#include "CommData.h"

struct MotorDriveState
{
	DriveDir DriveDirection = {};
};

class MotorDriveController : public DeviceController<MotorDriveState>
{
public:
	explicit MotorDriveController();
	virtual ~MotorDriveController() override;

protected:
	virtual void write(const MotorDriveState& state) override;
	virtual MotorDriveState getDefaultState() const override;
	virtual void sendState(const MotorDriveState& state) const override;
	virtual void processEvent(const Event& event) override;

private:
	class MotorControl* motorControl = nullptr;
};

#endif //PERSE_ROVER_MOTORDRIVECONTROLLER_H