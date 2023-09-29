#ifndef PERSE_ROVER_SHIFTREG_H
#define PERSE_ROVER_SHIFTREG_H

#include <array>
#include "AW9523.h"

class ShiftReg {
public:
	ShiftReg(AW9523& aw9523);
	void scan();

	bool get(uint8_t pin);

private:
	AW9523& aw9523;
	std::array<bool, 16> states;
};


#endif //PERSE_ROVER_SHIFTREG_H
