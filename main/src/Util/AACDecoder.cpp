#include "AACDecoder.hpp"
#include "esp_log.h"
#include <cstring>

static const char* TAG = "AACDecoder";

AACDecoder::AACDecoder(const std::string& path) : file(path){
	if(!file.is_open()){
		ESP_LOGE(TAG, "Failed to open file %s", path.c_str());
		return;
	}

	decoder = AACInitDecoder();
	if(decoder == nullptr){
		ESP_LOGE(TAG, "Libhelix AAC decoder failed to initialize.");
		return;
	}else{
		int16_t temp[BufferSize];
		getData(temp);
		AACFreeDecoder(decoder);
		decoder = AACInitDecoder();
	}

	if(decoder == nullptr){
		ESP_LOGE(TAG, "Libhelix AAC decoder failed to initialize.");
		return;
	}

	fillBuffer = (char*) malloc(AacDecodeMinInputSize);
	dataBuffer = (char*) malloc(BufferSize);

	file.seekg(SEEK_END);
	const size_t fileSize = file.tellg();
	file.seekg(0);

	frameCount = fileSize / AacReadChunk;
}

AACDecoder::~AACDecoder(){
	file.close();
	AACFreeDecoder(decoder);

	delete fillBuffer;
	delete dataBuffer;
}

bool AACDecoder::getData(int16_t* buffer){
	if(!file.is_open()){
		return false;
	}

	if(decoder == nullptr){
		return false;
	}

	if(buffer == nullptr){
		return false;
	}

	file.read(fillBuffer, AacDecodeMinInputSize);
	const size_t countRead = file.gcount();

	if(countRead < AacDecodeMinInputSize){
		return false;
	}

	uint8_t* data = (uint8_t*) fillBuffer;
	int bytesLeft = AacDecodeMinInputSize;

	if(int ret = AACDecode(decoder, &data, &bytesLeft, reinterpret_cast<short *>(dataBuffer))){
		ESP_LOGE(TAG, "AAC decoding error %d", ret);
		return false;
	}

	if(bytesLeft != 0){
		ESP_LOGE(TAG, "AAC decoding error, bytes left: %d", bytesLeft);
		return false;
	}

	AACFrameInfo frameInfo;
	AACGetLastFrameInfo(decoder, &frameInfo);

	bitrate = frameInfo.bitRate;
	channels = frameInfo.nChans;

	memcpy(buffer, dataBuffer, BufferSize);

	return true;
}

uint32_t AACDecoder::getBitrate() const{
	return bitrate;
}

uint32_t AACDecoder::getChannelNum() const{
	return channels;
}

uint32_t AACDecoder::getFrameCount() const{
	return frameCount;
}
