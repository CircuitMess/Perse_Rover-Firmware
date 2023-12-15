#ifndef PERSE_ROVER_AACDECODER_HPP
#define PERSE_ROVER_AACDECODER_HPP

#include <vector>
#include <string>
#include <fstream>
#include <aacdec.h>

class AACDecoder{
public:
	explicit AACDecoder(const std::string& path);
	virtual ~AACDecoder();

	size_t getData(int16_t* buffer, size_t size);

private:
	static constexpr size_t BufferSamples = 256;
	static constexpr size_t SampleRate = 24000;
	static constexpr size_t BytesPerSample = 2;
	static constexpr size_t NumberOfChannels = 1;
	static constexpr size_t BufferSize = BufferSamples * BytesPerSample * NumberOfChannels;
	static constexpr size_t AacReadBuffer = 1024 * 8;
	static constexpr size_t AacReadChunk = 1024 * 2;
	static constexpr size_t AacDecodeMinInputSize = 1024;
	static constexpr size_t AacOutBufferSize = 1024 * 4;

	HAACDecoder decoder = nullptr;
	std::ifstream file;
	int bytesRemaining = 0;

	std::vector<char> fillBuffer;
	std::vector<char> dataBuffer;
};

#endif //PERSE_ROVER_AACDECODER_HPP
