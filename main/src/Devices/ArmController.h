#ifndef PERSE_ROVER_ARMCONTROLLER_H
#define PERSE_ROVER_ARMCONTROLLER_H

#include <glm.hpp>
#include "DeviceController.h"
#include "CommData.h"
#include "Util/Easer.h"

struct ArmState
{
	ArmPos Position = -1;
	ArmPinch Pinch = -1;
};

class ArmController : public DeviceController<ArmState>
{
public:
	ArmController();
	virtual ~ArmController() override;

protected:
	virtual void write(const ArmState& state) override;
	virtual ArmState getDefaultState() const override;
	virtual void sendState(const ArmState& state, bool local) const override;
	virtual void processEvent(const Event& event) override;

private:
	class Servo* positionServo = nullptr;
	Servo* pinchServo = nullptr;
	static constexpr glm::vec<2, uint8_t> positionLimits = {0, 80};
	static constexpr glm::vec<2, uint8_t> pinchLimits = {30, 55};

	Easer posEase;
	Easer pinchEase;
};

#endif //PERSE_ROVER_ARMCONTROLLER_H