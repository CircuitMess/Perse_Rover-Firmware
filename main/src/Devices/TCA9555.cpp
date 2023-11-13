#include "TCA9555.h"
#include <esp_log.h>

#define REG_INPUT (0x00)
#define REG_OUTPUT (0x02)
#define REG_POLARITY (0x04)
#define REG_DIR (0x06)

#define IT(pin) ((pin) <= 7 ? 0 : 1)
#define REG(reg, pin) ((reg) + IT(pin))
#define BIT(pin) ((pin) <= 7 ? (pin) : (pin) - 8)
#define MASK(pin) (1 << BIT(pin))

static const char* TAG = "TCA9555";

TCA9555::TCA9555(I2C& i2C, const uint8_t addr) : i2c(i2C), Addr(addr){
	if(i2c.probe(Addr) != ESP_OK){
		ESP_LOGE(TAG, "Can't probe device");
		abort();
	}

	reset();
}

void TCA9555::reset(){
	i2c.write(Addr, { REG_DIR, 0xff, 0xff }); // All pins to input
	i2c.write(Addr, { REG_POLARITY, 0x00, 0x00 }); // Turn off polarity inversion for all pins
	regs = Regs();
}

void TCA9555::pinMode(uint8_t pin, TCA9555::PinMode mode){
	if(pin >= 16) return;

	const uint8_t it = IT(pin);
	const uint8_t mask = MASK(pin);
	const uint8_t regDir = REG(REG_DIR, pin);
	uint8_t& intRegDir = regs.dir[it];

	if(mode == OUT){
		intRegDir = intRegDir & ~mask;
	}else{
		intRegDir = intRegDir | mask;
	}

	ESP_ERROR_CHECK(i2c.writeReg(Addr, regDir, intRegDir));
}

bool TCA9555::read(uint8_t pin){
	if(pin >= 16) return false;

	const uint8_t reg = REG(REG_INPUT, pin);

	uint8_t regVal;
	ESP_ERROR_CHECK(i2c.readReg(Addr, reg, regVal));

	return regVal & MASK(pin);
}

uint16_t TCA9555::readAll(){
	uint8_t val[2];
	ESP_ERROR_CHECK(i2c.write_read(Addr, REG_INPUT, val, 2));
	return (val[1] << 8) | val[0];
}

void TCA9555::write(uint8_t pin, bool state){
	if(pin >= 16) return;

	const uint8_t reg = REG(REG_OUTPUT, pin);
	const uint8_t mask = MASK(pin);
	uint8_t& intReg = regs.output[IT(pin)];

	if(state){
		intReg |= mask;
	}else{
		intReg &= ~mask;
	}

	ESP_ERROR_CHECK(i2c.writeReg(Addr, reg, intReg));
}
