#include "Camera.h"
#include <Pins.hpp>
#include <driver/i2c.h>
#include <esp_log.h>
#include <esp_timer.h>

static const char* TAG = "Camera";

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

esp_err_t Camera::init(bool horizontalFlip){
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

	config.xclk_freq_hz = 12400000;

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

	if(sensor->id.PID == OV3660_PID){
		esp_camera_deinit();

		if(format == PIXFORMAT_JPEG){
			config.pixel_format = PIXFORMAT_RGB565;
			ManualJPGEncoding = true;
		}

		err = esp_camera_init(&config);
		if(err == ESP_ERR_NOT_FOUND){
			return err;
		} else if(err != ESP_OK){
			printf("Camera init failed with error 0x%x: %s\n", err, esp_err_to_name(err));
			return err;
		}

		sensor = esp_camera_sensor_get();
	}


	sensor->set_hmirror(sensor, horizontalFlip);
	sensor->set_vflip(sensor, horizontalFlip);

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

	logSensorTiming(sensor, config.xclk_freq_hz);

	inited = true;
	failedFrames = 0;
	resetStats();

	return ESP_OK;
}

// Dumps the GC2145 timing-relevant registers once after init and derives the
// number of sensor pixel clocks per frame, so the measured VSYNC period
// (see cam_hal "VSYNC:" log) can be turned into the actual sensor pixel clock:
//   pixel_clock [Hz] = clocks_per_frame / frame_period [s]
// Datasheet 7.1.1: row_time = HB + sh_delay + win_width + 4,
//                  frame     = VB + win_height + 8 rows (or exposure-limited).
void Camera::logSensorTiming(sensor_t* sensor, uint32_t xclk){
	if(sensor == nullptr || sensor->get_reg == nullptr || sensor->set_reg == nullptr) return;

	ESP_LOGI(TAG, "Sensor PID 0x%04x, XCLK %lu Hz, framesize %d (%ux%u), pixformat %d",
			 sensor->id.PID, (unsigned long) xclk, (int) sensor->status.framesize,
			 resolution[sensor->status.framesize].width, resolution[sensor->status.framesize].height,
			 (int) sensor->pixformat);

	if(sensor->id.PID != GC2145_PID) return;

	auto rd = [sensor](int reg) -> uint32_t {
		const int v = sensor->get_reg(sensor, reg, 0xff);
		return v < 0 ? 0 : (uint32_t) v;
	};
	auto rd16 = [&rd](int regH, int regL) -> uint32_t {
		return (rd(regH) << 8) | rd(regL);
	};

	sensor->set_reg(sensor, 0xfe, 0x07, 0x00); // page 0

	const uint32_t pllMode1 = rd(0xf7);
	const uint32_t pllMode2 = rd(0xf8);
	const uint32_t cmMode = rd(0xf9);
	const uint32_t clkDiv = rd(0xfa);
	const uint32_t scalar = rd(0xfd);

	const uint32_t exposure = rd16(0x03, 0x04) & 0x1fff;
	const uint32_t hb = rd16(0x05, 0x06) & 0x0fff;
	const uint32_t vb = rd16(0x07, 0x08) & 0x1fff;
	const uint32_t rowStart = rd16(0x09, 0x0a) & 0x07ff;
	const uint32_t colStart = rd16(0x0b, 0x0c) & 0x07ff;
	const uint32_t winH = rd16(0x0d, 0x0e) & 0x07ff;
	const uint32_t winW = rd16(0x0f, 0x10) & 0x07ff;
	const uint32_t shDelay = ((rd(0x11) & 0x03) << 8) | rd(0x12);

	const uint32_t cropEn = rd(0x90);
	const uint32_t outY = rd16(0x91, 0x92) & 0x07ff;
	const uint32_t outX = rd16(0x93, 0x94) & 0x07ff;
	const uint32_t outH = rd16(0x95, 0x96) & 0x07ff;
	const uint32_t outW = rd16(0x97, 0x98) & 0x07ff;
	const uint32_t subsample = rd(0x99);
	const uint32_t subMode = rd(0x9a);

	const uint32_t rowClocks = hb + shDelay + winW + 4;
	const uint32_t frameRows = vb + winH + 8;
	const uint32_t frameRowsExp = exposure + 8; // applies when exposure > VB + win_height
	const uint64_t clocksPerFrame = (uint64_t) rowClocks * (uint64_t) (frameRows > frameRowsExp ? frameRows : frameRowsExp);

	ESP_LOGI(TAG, "GC2145 PLL: 0xf7=0x%02lx 0xf8=0x%02lx (mult %lu) 0xf9=0x%02lx 0xfa=0x%02lx (divide_by %lu) 0xfd=0x%02lx",
			 (unsigned long) pllMode1, (unsigned long) pllMode2, (unsigned long) (pllMode2 & 0x3f),
			 (unsigned long) cmMode, (unsigned long) clkDiv, (unsigned long) (clkDiv >> 4), (unsigned long) scalar);
	ESP_LOGI(TAG, "GC2145 CISCTL: window %lux%lu @ col %lu row %lu, HB %lu, VB %lu, sh_delay %lu, exposure %lu rows",
			 (unsigned long) winW, (unsigned long) winH, (unsigned long) colStart, (unsigned long) rowStart,
			 (unsigned long) hb, (unsigned long) vb, (unsigned long) shDelay, (unsigned long) exposure);
	ESP_LOGI(TAG, "GC2145 ISP: crop_en 0x%02lx, out window %lux%lu @ x %lu y %lu, subsample 0x%02lx (row 1/%lu, col 1/%lu), mode 0x%02lx",
			 (unsigned long) cropEn, (unsigned long) outW, (unsigned long) outH, (unsigned long) outX, (unsigned long) outY,
			 (unsigned long) subsample, (unsigned long) (subsample >> 4), (unsigned long) (subsample & 0x0f), (unsigned long) subMode);

	// Plain-language summary of the pipeline. The CISCTL window carries 16 dummy
	// columns and 8 dummy rows on top of the signal pixels (esp32-camera sets
	// win = signal + 16 x signal + 8), so the visible crop is the window minus those.
	constexpr uint32_t SensorW = 1600, SensorH = 1200;
	const uint32_t cropW = winW > 16 ? winW - 16 : winW;
	const uint32_t cropH = winH > 8 ? winH - 8 : winH;
	const uint32_t ratioRow = (subsample >> 4) ? (subsample >> 4) : 1;
	const uint32_t ratioCol = (subsample & 0x0f) ? (subsample & 0x0f) : 1;
	const bool noCrop = cropW >= SensorW && cropH >= SensorH;
	const bool noSubsample = ratioRow == 1 && ratioCol == 1;

	if(noCrop){
		ESP_LOGI(TAG, "GC2145 pipeline: full sensor %lux%lu, no cropping (100%% field of view)", (unsigned long) SensorW, (unsigned long) SensorH);
	}else{
		ESP_LOGI(TAG, "GC2145 pipeline: cropping from %lux%lu to %lux%lu at col %lu row %lu (%lu%% x %lu%% of the field of view)",
				 (unsigned long) SensorW, (unsigned long) SensorH, (unsigned long) cropW, (unsigned long) cropH,
				 (unsigned long) colStart, (unsigned long) rowStart,
				 (unsigned long) (cropW * 100 / SensorW), (unsigned long) (cropH * 100 / SensorH));
	}
	if(noSubsample){
		ESP_LOGI(TAG, "GC2145 pipeline: no subsampling, output %lux%lu", (unsigned long) outW, (unsigned long) outH);
	}else{
		ESP_LOGI(TAG, "GC2145 pipeline: subsampling %lux%lu by 1/%lu (cols) x 1/%lu (rows) to %lux%lu, ISP outputs %lux%lu",
				 (unsigned long) cropW, (unsigned long) cropH, (unsigned long) ratioCol, (unsigned long) ratioRow,
				 (unsigned long) (cropW / ratioCol), (unsigned long) (cropH / ratioRow),
				 (unsigned long) outW, (unsigned long) outH);
	}
	ESP_LOGI(TAG, "GC2145 timing: %lu clocks/row x %lu rows (exposure-limited: %lu) = %llu clocks/frame",
			 (unsigned long) rowClocks, (unsigned long) frameRows, (unsigned long) frameRowsExp, (unsigned long long) clocksPerFrame);
	ESP_LOGI(TAG, "GC2145 timing: pixel clock = %llu / measured VSYNC period. At pixel clock == XCLK (%lu Hz) that is %.2f fps",
			 (unsigned long long) clocksPerFrame, (unsigned long) xclk, (double) xclk / (double) clocksPerFrame);
}

void Camera::resetStats(){
	statsStart = esp_timer_get_time();
	lastFrameTs = 0;
	statsFrames = 0;
	statsFailed = 0;
	statsDtSum = 0;
	statsDtMin = 0;
	statsDtMax = 0;
}

// Per-frame accounting. The driver stamps each frame at its VSYNC start, so the
// gap between consecutive delivered frames is k * sensor_period, k >= 1 (frames
// the app did not pick up in time are dropped by CAMERA_GRAB_LATEST). The min
// gap therefore approximates the sensor period, the mean is the app's rate.
void Camera::accountFrame(const camera_fb_t* fb){
	const int64_t now = esp_timer_get_time();

	if(fb == nullptr){
		statsFailed++;
	}else{
		statsFrames++;
		const int64_t ts = (int64_t) fb->timestamp.tv_sec * 1000000LL + fb->timestamp.tv_usec;
		if(lastFrameTs != 0 && ts > lastFrameTs){
			const int64_t dt = ts - lastFrameTs;
			statsDtSum += dt;
			if(statsDtMin == 0 || dt < statsDtMin) statsDtMin = dt;
			if(dt > statsDtMax) statsDtMax = dt;
		}
		lastFrameTs = ts;
	}

	const int64_t elapsed = now - statsStart;
	if(elapsed < StatsPeriodUs) return;

	const uint32_t intervals = statsFrames > 0 ? statsFrames - 1 : 0;
	ESP_LOGI(TAG, "Frames: %lu delivered, %lu failed in %lld ms -> app %.2f fps | frame gap avg %.2f ms, min %.2f ms (~sensor period, %.2f fps), max %.2f ms",
			 (unsigned long) statsFrames, (unsigned long) statsFailed, (long long) (elapsed / 1000),
			 1000000.0 * (double) statsFrames / (double) elapsed,
			 intervals ? (double) statsDtSum / 1000.0 / (double) intervals : 0.0,
			 (double) statsDtMin / 1000.0,
			 statsDtMin ? 1000000.0 / (double) statsDtMin : 0.0,
			 (double) statsDtMax / 1000.0);

	statsStart = now;
	statsFrames = 0;
	statsFailed = 0;
	statsDtSum = 0;
	statsDtMin = 0;
	statsDtMax = 0;
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
	accountFrame(frame);

	if(frame == nullptr){
		failedFrames++;

		if(failedFrames >= MaxFailedFrames){
			deinit();
		}
	}else{
		failedFrames = 0;
	}

	if(format == PIXFORMAT_JPEG && ManualJPGEncoding){
		frame2jpg(frame, 12, &buffJPG, &sizeJPG);
		frameJPG.height = frame->height;
		frameJPG.width = frame->width;
		frameJPG.buf = buffJPG;
		frameJPG.format = PIXFORMAT_JPEG;
		frameJPG.len = sizeJPG;

		return frame;
	}

	return frame;
}

void Camera::releaseFrame(){
	if(!inited) return;
	if(!frame) return;

	esp_camera_fb_return(frame);
	frame = nullptr;

	if(format == PIXFORMAT_JPEG && ManualJPGEncoding){
		free(buffJPG);
		buffJPG = nullptr;
		sizeJPG = 0;
		frameJPG = {};
	}
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
