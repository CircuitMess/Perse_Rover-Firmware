#ifndef PERSE_ROVER_HEADLIGHTSCONTROLLER_H
#define PERSE_ROVER_HEADLIGHTSCONTROLLER_H

#include "DeviceController.h"
#include "AW9523.h"

enum class HeadlightsMode : uint8_t
{
    Off,
    On
};

struct HeadlightsState
{
    HeadlightsMode Mode = HeadlightsMode::Off;
};

class HeadlightsController : public DeviceController<HeadlightsState>
{
public:
    HeadlightsController(AW9523* aw9523);
    virtual ~HeadlightsController() override = default;

protected:
    virtual void write(const HeadlightsState& state) override;
    virtual HeadlightsState getDefaultState() const override;
    virtual void sendState(const HeadlightsState& state) const override;
    virtual HeadlightsState processStateFromEvent(const Event& event) const override;

private:
    AW9523* aw9523 = nullptr;
};


#endif //PERSE_ROVER_HEADLIGHTSCONTROLLER_H
