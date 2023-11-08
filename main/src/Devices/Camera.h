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

	uint8_t getJpegQuality() const;
	void setJpegQuality(uint8_t quality);

	bool init();
	void deinit();
	bool isInited();

private:
	bool inited = false;
	framesize_t resWait = FRAMESIZE_QQVGA;
	pixformat_t formatWait = PIXFORMAT_JPEG;
	uint8_t jpegQualityWait = 12;

	camera_fb_t* frame = nullptr;

	framesize_t res = FRAMESIZE_INVALID;
	pixformat_t format = PIXFORMAT_RGB444;
	uint8_t jpegQuality = 0;

	static constexpr int MaxFailedFrames = 2;
	int failedFrames = 0;

	I2C& i2c;
};


#endif //PERSE_ROVER_CAMERA_H
