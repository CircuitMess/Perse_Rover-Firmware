#include "Feed.h"
#include <esp_log.h>
#include "Services/Comm.h"
#include "Util/stdafx.h"

const char* tag = "Feed";

Feed::Feed(I2C& i2c) : SleepyThreaded(50, "Feed", 4 * 1024), queue(10),
					   frameSendingThread(50, [this](){ this->sendFrame(); }, "FrameSending", 12 * 1024),
					   communicationQueue(10), txBuf(static_cast<uint8_t*>(malloc(TxBufSize))){
	memset(txBuf, 0, TxBufSize);

	Events::listen(Facility::TCP, &queue);
	Events::listen(Facility::Comm, &queue);

	camera = std::make_unique<Camera>(i2c);
	markerScanner = std::make_unique<MarkerScanner>(120, 160);

	start();
	frameSendingThread.start();
}

Feed::~Feed(){
	frameSendingThread.stop();

	if(camera != nullptr){
		camera->releaseFrame();
	}

	stop(0);
	queue.unblock();
	while(running()){
		delayMillis(1);
	}

	Events::unlisten(&queue);

	free(txBuf);
}

void Feed::sleepyLoop(){
	::Event event{};
	if(queue.get(event, portMAX_DELAY)){
		if(event.facility == Facility::TCP){
			if(const TCPServer::Event* tcpEvent = (TCPServer::Event*) event.data){
				if(tcpEvent->status == TCPServer::Event::Status::Disconnected){
					EventData data;
					data.type = EventData::FeedQualityChange;
					data.feedQuality = 0;

					communicationQueue.post(data, portMAX_DELAY);
				}
			}
		}else if(event.facility == Facility::Comm){
			if(const Comm::Event* commEvent = (Comm::Event*) event.data){
				if(commEvent->type == CommType::FeedQuality){
					EventData data;
					data.type = EventData::FeedQualityChange;
					data.feedQuality = std::clamp(commEvent->feedQuality, (uint8_t) 0, (uint8_t) 30);

					communicationQueue.post(data, portMAX_DELAY);
				}else if(commEvent->type == CommType::ScanMarkers){
					EventData data;
					data.type = EventData::ScanningEnableChange;
					data.isScanningEnabled = commEvent->scanningEnable;

					communicationQueue.post(data, portMAX_DELAY);
				}
			}
		}

		free(event.data);
	}
}

void IRAM_ATTR Feed::sendFrame(){
	for(EventData data; communicationQueue.get(data, 0);){
		if(data.type == EventData::None){
			continue;
		}

		if(data.type == EventData::FeedQualityChange){
			feedQuality = data.feedQuality;
		}else if(data.type == EventData::ScanningEnableChange){
			isScanningEnabled = data.isScanningEnabled;
		}
	}

	camera->setFormat(PIXFORMAT_RGB565);

	if(feedQuality == 0 && !isScanningEnabled){
		camera->deinit();
		return;
	}else{
		camera->init();
	}

	camera_fb_t* frameData = camera->getFrame();
	if(frameData == nullptr || frameData->buf == nullptr || frameData->len == 0){
		camera->releaseFrame();
		return;
	}

	DriveInfo driveInfo;

	if(isScanningEnabled){
		if(camera == nullptr || markerScanner == nullptr){
			return;
		}

		markerScanner->process(frameData->buf, driveInfo);

		if(driveInfo.markerInfo.action != oldAction){
			oldAction = driveInfo.markerInfo.action;

			Events::post(Facility::Feed,
						 Event{ .type = EventType::MarkerScanned, .markerAction = driveInfo.markerInfo.action });
		}
	}

	if(camera == nullptr){
		camera->releaseFrame();
		return;
	}

	if(!frame2jpg(frameData, 30/*std::clamp((uint8_t) feedQuality, (uint8_t) 0, (uint8_t) 12)*/,
				  (uint8_t**) (&driveInfo.frame.data), &driveInfo.frame.size)){
		ESP_LOGE(tag, "frame2jpg conversion failed.");
		camera->releaseFrame();
		return;
	}

	const size_t frameSize = driveInfo.size();
	const size_t sendSize = frameSize + sizeof(FrameHeader) + sizeof(FrameTrailer) + sizeof(size_t) * 2;

	if(sendSize > TxBufSize){
		ESP_LOGW(tag, "Data frame buffer larger than send buffer. %zu > %zu\n", sendSize, TxBufSize);
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
		shiftedFrame[FrameSizeShift[i]] = ((uint8_t*) &frameSize)[i];
	}
	addData(FrameHeader, sizeof(FrameHeader));
	addData(&frameSize, sizeof(size_t));
	addData(shiftedFrame, sizeof(size_t));
	driveInfo.toData(txBuf + cursor);
	cursor += frameSize;
	addData(FrameTrailer, sizeof(FrameTrailer));

	size_t sent = 0;
	while(sent < sendSize){
		const size_t sending = std::min((size_t) CONFIG_TCP_MSS, sendSize - sent);
		udp.write(txBuf + sent, sending);
		sent += sending;
	}

	camera->releaseFrame();
}