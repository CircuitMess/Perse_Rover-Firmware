#ifndef PERSE_ROVER_RGBMODULE_H
#define PERSE_ROVER_RGBMODULE_H

#include "Services/Modules.h"
#include "Devices/WS2812B.h"

class RGBModule : private SleepyThreaded{
public:
	RGBModule(ModuleBus bus);
	~RGBModule() override;

	void setAll(glm::vec<3, uint8_t> color);
	void setPixel(glm::vec<3, uint8_t> color, uint32_t index);

	void push();

protected:
	virtual void sleepyLoop() override;

private:
	WS2812B rgb;
};


#endif //PERSE_ROVER_RGBMODULE_H
