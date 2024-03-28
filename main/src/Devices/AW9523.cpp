#include "AW9523.h"
#include "Util/stdafx.h"
#include <esp_log.h>

#define REG_RESET 0x7F
#define REG_ID 0x10
#define REG_CONF 0x11
#define REG_INPUT 0x00
#define REG_OUTPUT 0x02
#define REG_DIR 0x04
#define REG_INTR 0x06
#define REG_MODE 0x12
#define REG_DIM 0x20

#define VAL_RESET 0x00
#define VAL_ID 0x23

#define CFG_MASK (0b00010011)

#define IT(pin) ((pin) <= 7 ? 0 : 1)
#define REG(reg, pin) ((reg) + IT(pin))
#define BIT(pin) ((pin) <= 7 ? (pin) : (pin) - 8)
#define MASK(pin) (1 << BIT(pin))

static const char* TAG = "AW9523";

const uint8_t AW9523::dimmap[16] = { 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 12, 13, 14, 15 };

AW9523::AW9523(I2C& i2c, uint8_t addr) : i2c(i2c), Addr(addr){
	if(i2c.probe(Addr) != ESP_OK){
		ESP_LOGE(TAG, "Transmission error");
		return;
	}

	// Reset
	writeReg(REG_RESET, VAL_RESET);
	regs = Regs();

	delayMicros(50);

	uint8_t id = readReg(REG_ID);
	if(id != VAL_ID){
		ESP_LOGE(TAG, "ID missmatch: expected %d, got %d", VAL_ID, id);
		return;
	}

	// All input
	regs.dir[0] = regs.dir[1] = 0xff;
	writeReg(REG_DIR, regs.dir, 2);

	// Push-pull mode for port 0
	regs.conf |= 0b00010000;
	writeReg(REG_CONF, regs.conf);

	// Disable interrupts
	regs.intr[0] = regs.intr[1] = 0xff;
	writeReg(REG_INTR, regs.intr, 2);
}

void AW9523::resetDimOutputs(){
	for(int i = 0; i < 16; ++i){
		dim(i, 0);
	}
}

void AW9523::pinMode(uint8_t pin, AW9523::PinMode mode){
	if(pin >= 16) return;

	const uint8_t it = IT(pin);
	const uint8_t mask = MASK(pin);
	const uint8_t regDir = REG(REG_DIR, pin);
	const uint8_t regMode = REG(REG_MODE, pin);
	uint8_t& intRegDir = regs.dir[it];
	uint8_t& intRegMode = regs.mode[it];

	if(mode == LED){
		intRegMode = intRegMode & ~mask;
		writeReg(regMode, intRegMode);
	}else{
		intRegMode = intRegMode | mask;
		writeReg(regMode, intRegMode);

		if(mode == OUT){
			intRegDir = intRegDir & ~mask;
		}else{
			intRegDir = intRegDir | mask;
		}

		writeReg(regDir, intRegDir);
	}
}

bool AW9523::read(uint8_t pin){
	if(pin >= 16) return false;

	const uint8_t reg = REG(REG_INPUT, pin);
	return readReg(reg) & MASK(pin);
}

void AW9523::write(uint8_t pin, bool state){
	if(pin >= 16) return;

	const uint8_t reg = REG(REG_OUTPUT, pin);
	const uint8_t mask = MASK(pin);
	uint8_t& intReg = regs.output[IT(pin)];

	if(state){
		intReg |= mask;
	}else{
		intReg &= ~mask;
	}

	writeReg(reg, intReg);
}

void AW9523::dim(uint8_t pin, uint8_t factor){
	if(pin >= 16) return;

	pin = dimmap[pin];
	regs.dim[pin] = factor;
	writeReg(REG_DIM + pin, factor);
}

void AW9523::setInterrupt(uint8_t pin, bool enabled){
	if(pin >= 16) return;

	const uint8_t reg = REG(REG_INTR, pin);
	const uint8_t mask = MASK(pin);
	uint8_t& intReg = regs.intr[IT(pin)];

	if(enabled){
		intReg |= mask;
	}else{
		intReg &= ~mask;
	}

	writeReg(reg, intReg);
}

void AW9523::setCurrentLimit(AW9523::CurrentLimit limit){
	const uint8_t mask = 0b00000011;
	uint8_t& conf = regs.conf;
	conf = (conf & ~mask) | (limit & mask);
	writeReg(REG_CONF, conf & CFG_MASK);
}

uint8_t AW9523::readReg(uint8_t reg) const{
	uint8_t data;
	i2c.readReg(Addr, reg, data);
	return data;
}

void AW9523::writeReg(uint8_t reg, const uint8_t* data, size_t size) const{
	ESP_ERROR_CHECK(i2c.writeReg(Addr, reg, data, size));
}

void AW9523::writeReg(uint8_t reg, uint8_t data) const{
	ESP_ERROR_CHECK(i2c.writeReg(Addr, reg, data));
}
