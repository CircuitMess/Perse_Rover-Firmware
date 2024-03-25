#include "AACDecoder.h"
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

size_t AACDecoder::getData(SampleType* buffer, size_t bytes){
	if(decoder == nullptr){
		ESP_LOGD(TAG, "early return, decoder == nullptr");
		return 0;
	}

	if(buffer == nullptr){
		ESP_LOGD(TAG, "early return, buffer == nullptr");
		return 0;
	}

	if(bytes == 0){
		ESP_LOGD(TAG, "early return, bytes == 0");
		return 0;
	}

	size_t bytesTransferred = 0;

	if(!dataBuffer.empty()){
		memcpy(buffer, dataBuffer.data(), std::min(dataBuffer.size(), bytes));
		bytesTransferred += dataBuffer.size();
		dataBuffer.clear();
	}

	while(bytesTransferred < bytes){
		ESP_LOGD(TAG, "bytesRemaining: %d", bytesRemaining);

		if(fillBuffer.size() < FileReadThreshold && file){
			fillBuffer.resize(fillBuffer.size() + FileReadChunkSize);
			file.read(fillBuffer.data() + fillBuffer.size() - FileReadChunkSize, FileReadChunkSize);
			bytesRemaining += file.gcount();
			ESP_LOGD(TAG, "fillBuffer resized, bytesRemaining: %d", bytesRemaining);
		}

		if(bytesRemaining <= 0){
			break;
		}

		unsigned char* inBuffer = (unsigned char*) fillBuffer.data();

		const int bytesRemainingBefore = bytesRemaining;

		dataBuffer.resize(dataBuffer.size() + DecodeOutBufferSize, 0);
		if(int ret = AACDecode(decoder, &inBuffer, &bytesRemaining, reinterpret_cast<SampleType*>(dataBuffer.data() + dataBuffer.size() - DecodeOutBufferSize))){
			ESP_LOGE(TAG, "AAC decoding error %d", ret);
			return 0;
		}

		const int bytesDecoded = bytesRemainingBefore - bytesRemaining;
		ESP_LOGD(TAG, "decoded: %d", bytesDecoded);

		fillBuffer.erase(fillBuffer.begin(), fillBuffer.begin() + bytesDecoded);

		const int bytesToTransfer = std::min(dataBuffer.size(), bytes - bytesTransferred);
		memcpy((buffer + bytesTransferred / SampleSize), dataBuffer.data(), bytesToTransfer);
		dataBuffer.erase(dataBuffer.begin(), dataBuffer.begin() + bytesToTransfer);

		bytesTransferred += bytesToTransfer;
		ESP_LOGD(TAG, "bytesTransferred: %d", bytesTransferred);
	}

	return bytesTransferred;
}
