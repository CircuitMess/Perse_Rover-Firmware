#ifndef PERSE_ROVER_CAMERA_H
#define PERSE_ROVER_CAMERA_H

#include <esp_camera.h>
#include "AW9523.h"

class Camera {
public:
	Camera(I2C& i2c);
	virtual ~Camera();

	camera_fb_t* getFrame();
	void releaseFrame();

	void setRes(framesize_t res);
	framesize_t getRes() const;

	pixformat_t getFormat() const;
	void setFormat(pixformat_t format);

	esp_err_t init(bool horizontalFlip = false);
	void deinit();
	bool isInited();

private:
	bool inited = false;
	framesize_t resWait = FRAMESIZE_QQVGA;
	pixformat_t formatWait = PIXFORMAT_JPEG;

	camera_fb_t* frame = nullptr;
	uint8_t* buffJPG = nullptr;
	size_t sizeJPG = 0;
	camera_fb_t frameJPG = {};

	framesize_t res = FRAMESIZE_INVALID;
	pixformat_t format = PIXFORMAT_RGB444;

	static constexpr int MaxFailedFrames = 100;
	int failedFrames = 0;

	bool ManualJPGEncoding = false;

	I2C& i2c;

	// Frame-time instrumentation
	static constexpr int64_t StatsPeriodUs = 2000000;
	int64_t statsStart = 0;
	int64_t lastFrameTs = 0;
	uint32_t statsFrames = 0;
	uint32_t statsFailed = 0;
	int64_t statsDtSum = 0;
	int64_t statsDtMin = 0;
	int64_t statsDtMax = 0;
	void resetStats();
	void accountFrame(const camera_fb_t* fb);
	static void logSensorTiming(sensor_t* sensor, uint32_t xclk);
};


#endif //PERSE_ROVER_CAMERA_H
