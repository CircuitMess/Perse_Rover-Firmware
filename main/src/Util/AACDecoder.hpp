#ifndef PERSE_ROVER_AACDECODER_HPP
#define PERSE_ROVER_AACDECODER_HPP

#include <string>
#include <fstream>
#include <aacdec.h>

class AACDecoder{
public:
	explicit AACDecoder(const std::string& path);
	virtual ~AACDecoder();

	bool getData(int16_t* buffer);

	uint32_t getBitrate() const;
	uint32_t getChannelNum() const;
	uint32_t getFrameCount() const;

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

	uint32_t bitrate = 0;
	uint32_t channels = 0;
	uint32_t frameCount = 0;

	HAACDecoder decoder = nullptr;
	std::ifstream file;

	char* fillBuffer = nullptr;
	char* dataBuffer = nullptr;
};

#endif //PERSE_ROVER_AACDECODER_HPP
