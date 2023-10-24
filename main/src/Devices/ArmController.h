#ifndef PERSE_ROVER_ARMCONTROLLER_H
#define PERSE_ROVER_ARMCONTROLLER_H

#include "DeviceController.h"
#include "CommData.h"

struct ArmState
{
	ArmPos Position = -1;
	ArmPinch Pinch = -1;
};

class ArmController : DeviceController<ArmState>
{
public:
	ArmController();
	virtual ~ArmController() override;

protected:
	virtual void write(const ArmState& state) override;
	virtual ArmState getDefaultState() const override;
	virtual void sendState(const ArmState& state) const override;
	virtual void processEvent(const Event& event) override;

private:
	class Servo* positionServo = nullptr;
	Servo* pinchServo = nullptr;
};

#endif //PERSE_ROVER_ARMCONTROLLER_H