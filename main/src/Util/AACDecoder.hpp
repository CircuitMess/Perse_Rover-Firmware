#ifndef PERSE_ROVER_AACDECODER_HPP
#define PERSE_ROVER_AACDECODER_HPP

#include <vector>
#include <string>
#include <fstream>
#include <aacdec.h>

class AACDecoder{
public:
	using SampleType = int16_t;

	explicit AACDecoder(const std::string& path);
	virtual ~AACDecoder();

	size_t getData(SampleType* buffer, size_t bytes);

private:
	static constexpr size_t FileReadThreshold = 1024;
	static constexpr size_t FileReadChunkSize = 1024;
	static constexpr size_t ChannelNumber = 1;
	static constexpr size_t SampleSize = sizeof(SampleType);
	static constexpr size_t DecodeOutBufferSize = ChannelNumber * 1024 * SampleSize; // Size of decoded data from one decode pass is 1024 samples per channel

	HAACDecoder decoder = nullptr;
	std::ifstream file;
	int bytesRemaining = 0;

	std::vector<char> fillBuffer;
	std::vector<char> dataBuffer;
};

#endif //PERSE_ROVER_AACDECODER_HPP
