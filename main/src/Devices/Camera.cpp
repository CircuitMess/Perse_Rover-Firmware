#include "Camera.h"
#include <Pins.hpp>
#include <driver/i2c.h>

Camera* Camera::instance = nullptr;

Camera::Camera(AW9523& aw9523) : aw9523(aw9523){
	instance = this;
}

Camera::~Camera(){
	instance = nullptr;
	deinit();
}

bool Camera::init(){
	if(resWait == res && formatWait == format && inited) return true;

	aw9523.write(CAM_PWDN, 0);

//	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
	i2c_config_t conf = {};
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = CAM_PIN_SIOD;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = CAM_PIN_SIOC;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = CONFIG_SCCB_CLK_FREQ;
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));

	if(inited){
		deinit();
	}

	format = formatWait;
	res = resWait;

	printf("Cam init size %d, format %d\n", res, format);

	camera_config_t config;
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer = LEDC_TIMER_0;

	config.pin_pwdn = -1;
	config.pin_reset = CAM_PIN_RESET;
	config.pin_xclk = CAM_PIN_XCLK;
	config.pin_sccb_sda = CAM_PIN_SIOD;
	config.pin_sccb_scl = CAM_PIN_SIOC;
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

	config.sccb_i2c_port = I2C_NUM_0;

	config.frame_size = res;
	config.pixel_format = format;
	config.fb_count = 2;
	config.fb_location = CAMERA_FB_IN_PSRAM;
	config.grab_mode = CAMERA_GRAB_LATEST;

	if(format == PIXFORMAT_JPEG){
		config.jpeg_quality = 28;
	}

	auto err = esp_camera_init(&config);
	if(err != ESP_OK){
		printf("Camera init failed with error 0x%x: %s\n", err, esp_err_to_name(err));
		return false;
	}

	sensor_t* sensor = esp_camera_sensor_get();
	sensor->set_hmirror(sensor, 1);
	sensor->set_vflip(sensor, 1);
	sensor->set_special_effect(sensor, 0);
	sensor->set_saturation(sensor, 0);

	if(res > FRAMESIZE_QQVGA){
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
	}

	inited = true;
	failedFrames = 0;

	i2c_config_t cfg = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = I2C_SDA,
		.scl_io_num = I2C_SCL,
		.sda_pullup_en = false,
		.scl_pullup_en = false
	};
	cfg.master.clk_speed = 400000;

	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &cfg));

//	gpio_set_direction((gpio_num_t) I2C_SDA, GPIO_MODE_INPUT);
//	gpio_set_direction((gpio_num_t) I2C_SCL, GPIO_MODE_INPUT);



	return true;
}

void Camera::deinit(){
	if(!inited) return;
	inited = false;

	if(frame){
		esp_camera_fb_return(frame);
		frame = nullptr;
	}

	esp_camera_deinit();

	aw9523.write(CAM_PWDN, 1);
}

camera_fb_t* Camera::getFrame(){
	if(!inited) return nullptr;
	if(frame) return nullptr;

	frame = esp_camera_fb_get();

	if(frame == nullptr){
		failedFrames++;
		if(failedFrames >= MaxFailedFrames){
			printf("Camera errored out\n");
			deinit();
		}
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

Camera* Camera::getInstance(){
	return instance;
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
