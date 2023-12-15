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
	}
}

AACDecoder::~AACDecoder(){
	file.close();
	AACFreeDecoder(decoder);
}

size_t AACDecoder::getData(int16_t* buffer, size_t size){
	if(!file){
		return 0;
	}

	if(decoder == nullptr){
		return 0;
	}

	if(buffer == nullptr){
		return 0;
	}

	size_t bytesTransfered = 0;

	if(!dataBuffer.empty()){
		memcpy(buffer, dataBuffer.data(), std::min(dataBuffer.size(), size * sizeof(int16_t)));
		bytesTransfered += dataBuffer.size();
		dataBuffer.clear();
	}

	while(bytesTransfered < size * sizeof(int16_t)){
		if(fillBuffer.size() < 512 && file){
			fillBuffer.resize(fillBuffer.size() + 1024);
			file.read(fillBuffer.data() + fillBuffer.size() - 1024, 1024);
			bytesRemaining += file.gcount();
		}

		if(bytesRemaining <= 0){
			break;
		}

		unsigned char* inBuffer = (unsigned char*) fillBuffer.data();

		const int bytesRemainingBefore = bytesRemaining;

		// TODO check if 1024 is actually correct. It should be, one decode run should equal to 1024 samples per channel, with one sample being two bytes, in this case there is only one, but I might be wrong
		dataBuffer.resize(dataBuffer.size() + 1024 * sizeof(int16_t));
		if(int ret = AACDecode(decoder, &inBuffer, &bytesRemaining, reinterpret_cast<short*>(dataBuffer.data() + dataBuffer.size() - 1024 * sizeof(int16_t )))){
			ESP_LOGE(TAG, "AAC decoding error %d", ret);
			return false;
		}

		const int bytesDecoded = bytesRemainingBefore - bytesRemaining;

		fillBuffer.erase(fillBuffer.begin(), fillBuffer.begin() + bytesDecoded);

		const int bytesToTransfer = std::min(bytesDecoded, (int)(size * sizeof(int16_t) - bytesTransfered));
		memcpy((buffer + bytesTransfered), dataBuffer.data(), bytesToTransfer);
		dataBuffer.erase(dataBuffer.begin(), dataBuffer.begin() + bytesToTransfer);

		bytesTransfered += bytesToTransfer;
	}

	return bytesTransfered;
}
