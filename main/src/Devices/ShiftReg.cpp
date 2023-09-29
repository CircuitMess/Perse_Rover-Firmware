#include "ShiftReg.h"
#include "Util/stdafx.h"
#include "Pins.hpp"

#define PERIOD 1
#define LH(pin) do { aw9523.write((pin), false); delayMicros(PERIOD); aw9523.write((pin), true); delayMicros(PERIOD); } while(0)
#define HL(pin) do { aw9523.write((pin), true); delayMicros(PERIOD); aw9523.write((pin), false); delayMicros(PERIOD); } while(0)


ShiftReg::ShiftReg(AW9523& aw9523) : aw9523(aw9523){
	aw9523.pinMode(EXP_SHIFT_DATA, AW9523::IN);
	aw9523.pinMode(EXP_SHIFT_CLOCK, AW9523::OUT);
	aw9523.pinMode(EXP_SHIFT_LATCH, AW9523::OUT);
}

void ShiftReg::scan(){
	aw9523.write(EXP_SHIFT_CLOCK, false);
	LH(EXP_SHIFT_LATCH);

	for(int i = 0; i < 16; i++){
		states[16 - i - 1] = aw9523.read(EXP_SHIFT_DATA);
		HL(EXP_SHIFT_CLOCK);
	}


	printf("scan: ");
	for(const auto& val : states){
		printf("%d ", (int) val);

	}
	printf("\n");
}

bool ShiftReg::get(uint8_t pin){
	return states[pin];
}


