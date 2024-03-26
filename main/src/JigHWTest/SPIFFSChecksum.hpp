#ifndef SPIFFSCHECKSUM_HPP
#define SPIFFSCHECKSUM_HPP

struct {
	const char* name;
	uint32_t sum;
} static const SPIFFSChecksums[] PROGMEM = {
	{ "/audioOn.wav", 13347341},
};

#endif //SPIFFSCHECKSUM_HPP
