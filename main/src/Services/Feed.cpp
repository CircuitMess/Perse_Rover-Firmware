#include "Feed.h"
#include <esp_log.h>
#include "Services/Comm.h"

const char* tag = "Feed";

Feed::Feed(I2C& i2c) : Threaded("Feed", 4 * 1024), queue(10),
				frameSendingThread(50, [this]() { this->sendFrame(); }, "FrameSending", 4 * 1024),
				txBuf(static_cast<uint8_t*>(malloc(TxBufSize))) {
	memset(txBuf, 0, TxBufSize);

	Events::listen(Facility::TCP, &queue);
	Events::listen(Facility::Comm, &queue);

	camera = std::make_unique<Camera>(i2c);

	start();
	frameSendingThread.start();
}

Feed::~Feed(){
	frameSendingThread.stop();
	free(txBuf);
	Events::unlisten(&queue);
}

void Feed::loop() {
	Event event{};
	if (!queue.get(event, portMAX_DELAY)) {
		return;
	}

	const uint8_t oldFeedQuality = feedQuality;

	if (event.facility == Facility::TCP) {
		if (const TCPServer::Event* tcpEvent = (TCPServer::Event*)event.data) {
			if (tcpEvent->status == TCPServer::Event::Status::Disconnected) {
				feedQuality = 0;
			}
		}
	}
	else if (event.facility == Facility::Comm) {
		if (const Comm::Event* commEvent = (Comm::Event*)event.data) {
			if (commEvent->type == CommType::FeedQuality) {
				feedQuality = std::clamp(commEvent->feedQuality, (uint8_t)0, (uint8_t)10);
			}
		}
	}

	free(event.data);

	if (feedQuality == oldFeedQuality) {
		return;
	}

	onFeedQualityChanged(oldFeedQuality);

	if (feedQuality != 0 && oldFeedQuality == 0) {
		onSendingStarted();
	}
	else if (feedQuality == 0 && oldFeedQuality != 0) {
		onSendingEnded();
	}
}

void Feed::sendFrame(){
	if (feedQuality == 0) {
		return;
	}

	if (camera == nullptr) {
		return;
	}

	if (!camera->isInited()) {
		return;
	}

	camera_fb_t* frameData = camera->getFrame();
	if (frameData == nullptr) {
		return;
	}

	if(frameData->buf == nullptr || frameData->len == 0){
		camera->releaseFrame();
		return;
	}

	DriveInfo frame{};
	frame.frame = {.size = frameData->len, .data = frameData->buf};

	auto frameSize = frame.size();
	auto sendSize = frameSize + sizeof(FrameHeader) + sizeof(FrameTrailer) + sizeof(size_t) * 2;
	if(sendSize > TxBufSize){
		ESP_LOGW(tag, "Data frame buffer larger than send buffer. %zu > %zu\n", sendSize, TxBufSize);
		frame.frame = {.size = 0, .data = nullptr};
		camera->releaseFrame();
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

	camera->releaseFrame();

	frame.frame = {.size = 0, .data = nullptr};
}

void Feed::onSendingStarted() {
	if (camera != nullptr) {
		if (!camera->isInited()) {
			camera->init();
		}
	}

	if (frameSendingThread.running()) {
		return;
	}

	frameSendingThread.start();
}

void Feed::onSendingEnded() {
	if (frameSendingThread.running()) {
		frameSendingThread.stop();
	}

	if (camera == nullptr) {
		return;
	}

	camera->deinit();
}

void Feed::onFeedQualityChanged(uint8_t oldFeedQuality) {

}
