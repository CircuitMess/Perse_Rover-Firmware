#include "Camera.h"
#include <Pins.hpp>
#include <driver/i2c.h>

Camera::Camera(I2C& i2c) : i2c(i2c){
	const gpio_config_t cfg = {
			.pin_bit_mask = 1ULL << CAM_PIN_PWDN,
			.mode = GPIO_MODE_OUTPUT
	};
	gpio_config(&cfg);
	gpio_set_level((gpio_num_t) CAM_PIN_PWDN, 1);
}

Camera::~Camera(){
	deinit();
}

esp_err_t Camera::init(){
	if(resWait == res && formatWait == format && inited) return ESP_OK;

	if(inited){
		deinit();
	}

	format = formatWait;
	res = resWait;

	camera_config_t config;
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer = LEDC_TIMER_0;

	config.pin_pwdn = -1;
	config.pin_reset = CAM_PIN_RESET;
	config.pin_xclk = CAM_PIN_XCLK;
	config.pin_d7 = CAM_PIN_D7;
	config.pin_d6 = CAM_PIN_D6;
	config.pin_d5 = CAM_PIN_D5;
	config.pin_d4 = CAM_PIN_D4;
	config.pin_d3 = CAM_PIN_D3;
	config.pin_d2 = CAM_PIN_D2;
	config.pin_d1 = CAM_PIN_D1;
	config.pin_d0 = CAM_PIN_D0;
	config.pin_vsync = CAM_PIN_VSYNC;
	config.pin_href = CAM_PIN_HREF;
	config.pin_pclk = CAM_PIN_PCLK;

	config.xclk_freq_hz = 14000000;

	config.sccb_i2c_port = i2c.getPort();
	config.pin_sccb_sda = -1;
	config.pin_sccb_scl = -1;

	config.frame_size = res;
	config.pixel_format = format;
	config.fb_count = 2;
	config.fb_location = CAMERA_FB_IN_PSRAM;
	config.grab_mode = CAMERA_GRAB_LATEST;

	if(format == PIXFORMAT_JPEG){
		config.jpeg_quality = 12;
	}

	gpio_set_level((gpio_num_t) CAM_PIN_PWDN, 0);

	auto lock = i2c.lockBus();

	auto err = esp_camera_init(&config);
	if(err == ESP_ERR_NOT_FOUND){
		return err;
	}else if(err != ESP_OK){
		printf("Camera init failed with error 0x%x: %s\n", err, esp_err_to_name(err));
		return err;
	}

	sensor_t* sensor = esp_camera_sensor_get();
	if(sensor == nullptr){
		return ESP_ERR_CAMERA_NOT_DETECTED;
	}

	sensor->set_hmirror(sensor, 0);
	sensor->set_vflip(sensor, 0);

	// sensor->set_saturation(sensor, 2);
	// sensor->set_awb_gain(sensor, 1);
	// sensor->set_wb_mode(sensor, 0);
	//sensor->set_exposure_ctrl(sensor, 0);
	sensor->set_gain_ctrl(sensor, 1);

	/*if(res > FRAMESIZE_QQVGA){
		sensor->set_brightness(sensor, -2);

		sensor->set_whitebal(sensor, 0);
		sensor->set_awb_gain(sensor, 0);
		sensor->set_wb_mode(sensor, 1);

		sensor->set_exposure_ctrl(sensor, 1);
		sensor->set_aec2(sensor, 0);
		sensor->set_ae_level(sensor, 0);
		sensor->set_aec_value(sensor, 300);
	}else{
		sensor->set_brightness(sensor, 1);
		sensor->set_contrast(sensor, 1);
		sensor->set_whitebal(sensor, 1);
	}*/

	inited = true;
	failedFrames = 0;

	return ESP_OK;
}

void Camera::deinit(){
	if(!inited) return;
	inited = false;

	if(frame){
		esp_camera_fb_return(frame);
		frame = nullptr;
	}

	{
		auto lock = i2c.lockBus();
		esp_camera_deinit();
	}

	gpio_set_level((gpio_num_t) CAM_PIN_PWDN, 1);
}

camera_fb_t* Camera::getFrame(){
	if(!inited) return nullptr;
	if(frame) return nullptr;

	frame = esp_camera_fb_get();

	if(frame == nullptr){
		failedFrames++;
/*		if(failedFrames >= MaxFailedFrames){
			printf("Camera errored out\n");
			deinit();
		}*/
	}

	return frame;
}

void Camera::releaseFrame(){
	if(!inited) return;
	if(!frame) return;

	esp_camera_fb_return(frame);
	frame = nullptr;
}

bool Camera::isInited(){
	return inited;
}

void Camera::setRes(framesize_t res){
	resWait = res;
}

framesize_t Camera::getRes() const{
	return res;
}

pixformat_t Camera::getFormat() const{
	if(format == PIXFORMAT_RGB565) return PIXFORMAT_RGB888;
	return format;
}

void Camera::setFormat(pixformat_t format){
	if(format == PIXFORMAT_RGB888){
		format = PIXFORMAT_RGB565;
	}

	formatWait = format;
}
