#include "Feed.h"
#include <esp_log.h>

const char* tag = "Feed";

Feed::Feed() : txBuf(static_cast<uint8_t*>(malloc(TxBufSize))){
	memset(txBuf, 0, TxBufSize);
}

Feed::~Feed(){
	free(txBuf);
}

void Feed::sendFrame(const DriveInfo& frame){
	auto frameSize = frame.size();
	auto sendSize = frameSize + sizeof(FrameHeader) + sizeof(FrameTrailer) + sizeof(size_t) * 2;
	if(sendSize > TxBufSize){
		ESP_LOGW(tag, "Data frame buffer larger than send buffer. %zu > %zu\n", sendSize, TxBufSize);
		return;
	}

	size_t cursor = 0;
	auto addData = [&cursor, this](const void* data, size_t size){
		memcpy(txBuf + cursor, data, size);
		cursor += size;
	};

	uint8_t shiftedFrame[4];
	for(uint8_t i = 0; i < 4; i++){
		shiftedFrame[FrameSizeShift[i]]  = ((uint8_t*)&frameSize)[i];
	}
	addData(FrameHeader, sizeof(FrameHeader));
	addData(&frameSize, sizeof(size_t));
	addData(shiftedFrame, sizeof(size_t));
	frame.toData(txBuf + cursor); cursor += frameSize;
	addData(FrameTrailer, sizeof(FrameTrailer));

	size_t sent = 0;
	while(sent < sendSize){
		size_t sending = std::min((size_t) CONFIG_TCP_MSS, sendSize - sent);
		udp.write(txBuf + sent, sending);
		sent += sending;
	}
}

