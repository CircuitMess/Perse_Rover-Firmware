#ifndef PERSE_ROVER_HEADLIGHTSCONTROLLER_H
#define PERSE_ROVER_HEADLIGHTSCONTROLLER_H

#include "DeviceController.h"
#include "CommData.h"
#include "AW9523.h"

struct HeadlightsState
{
	HeadlightsMode Mode = HeadlightsMode::Off;
};

class HeadlightsController : public DeviceController<HeadlightsState>
{
public:
	explicit HeadlightsController(AW9523& aw9523);
	virtual ~HeadlightsController() override = default;

protected:
	virtual void write(const HeadlightsState& state) override;
	virtual HeadlightsState getDefaultState() const override;
	virtual void sendState(const HeadlightsState& state) const override;
	virtual void processEvent(const Event& event) override;

private:
	AW9523& aw9523;
};

#endif //PERSE_ROVER_HEADLIGHTSCONTROLLER_H