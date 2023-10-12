#include "HeadlightsController.h"
#include "Pins.hpp"

HeadlightsController::HeadlightsController(AW9523* aw9523) : DeviceController(), aw9523(aw9523)
{
    if (aw9523 == nullptr)
    {
        return;
    }

    aw9523->pinMode(EXP_HEADLIGHT_1, AW9523::LED);
    aw9523->pinMode(EXP_HEADLIGHT_2, AW9523::LED);
}

void HeadlightsController::write(const HeadlightsState& state)
{
    if (aw9523 == nullptr)
    {
        // TODO: print error
        return;
    }

    if (state.Mode == HeadlightsMode::Off)
    {
        aw9523->dim(EXP_HEADLIGHT_1, 0);
        aw9523->dim(EXP_HEADLIGHT_2, 0);
    }
    else
    {
        aw9523->dim(EXP_HEADLIGHT_1, 255);
        aw9523->dim(EXP_HEADLIGHT_2, 255);
    }
}

HeadlightsState HeadlightsController::getDefaultState() const
{
    return HeadlightsState{};
}

void HeadlightsController::sendState(const HeadlightsState& state) const
{
    // TODO send state to mission control via comm
}

HeadlightsState HeadlightsController::processStateFromEvent(const Event& event) const
{
    // TODO: cast data of event into comm event and look at the data inside to determine how to create the state
    return HeadlightsState{};
}

