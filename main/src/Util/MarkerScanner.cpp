#include "MarkerScanner.h"
#include <glm.hpp>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include "ArucoValidator.h"

#undef EPS
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>

MarkerScanner::MarkerScanner(uint8_t frameWidth, uint8_t frameHeight) : width(frameWidth), height(frameHeight){
	smallData = std::unique_ptr<uint8_t>((uint8_t*) heap_caps_malloc(width * height, MALLOC_CAP_SPIRAM));
	bwData = std::unique_ptr<uint8_t>((uint8_t*) heap_caps_malloc(width * height, MALLOC_CAP_SPIRAM));

	small = cv::Mat(width, height, CV_8U, smallData.get());
	bw = cv::Mat(width, height, CV_8U, bwData.get());

	small.data = smallData.get();
	bw.data = bwData.get();

	scale = scaleMin + accuracy * (scaleMax - scaleMin);

	float floatBox = boxMin + accuracy * (scaleMax - scaleMin);
	int intBox = std::roundf(floatBox);

	if(intBox % 2 == 1){
		box = intBox;
	}else if(intBox % 2 == 0){
		box = intBox - 1;
	}else if((int) std::floor(box) % 2 == 0){
		box = std::ceil(box);
	}else{
		box = std::floor(box);
	}

	box = std::clamp(box, (int8_t) boxMin, (int8_t) boxMax);

	printf("Accuracy: %.1f, scale: %.2f, box: %d\n", accuracy, scale, (int) box);
}

bool MarkerScanner::process(const uint8_t* rawFrame, DriveInfo& driveInfo){
	if(rawFrame == nullptr){
		return false;
	}

	driveInfo.markerInfo.action = MarkerAction::None;
	driveInfo.markerInfo.markers.clear();

	std::vector<uint8_t> grayFrame(width * height);
	// Only for RGB565
	for(size_t i = 0; i < grayFrame.size(); ++i){
		uint16_t color = ((uint16_t*) rawFrame)[i];
		color = (color >> 8) | (color << 8);

		uint8_t r = (color & 0xF800) >> 11;
		uint8_t g = (color & 0x07E0) >> 5;
		uint8_t b = (color & 0x001F);

		r = std::clamp((r * 255) / 31, 0, 255);
		g = std::clamp((g * 255) / 63, 0, 255);
		b = std::clamp((b * 255) / 31, 0, 255);

		grayFrame[i] = (r + g + b) / 3;
	}

	const cv::Mat gray(width, height, CV_8U, grayFrame.data());

	cv::resize(gray, small, {}, scale, scale, cv::InterpolationFlags::INTER_LINEAR);
	GaussianBlur(small, small, cv::Size(3, 3), 0, 0);
	cv::adaptiveThreshold(small, bw, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, box - 2, 10);

	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(bw, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

	for(const std::vector<cv::Point>& contour: contours){
		std::vector<cv::Point> approx(4);
		if(!contourValid(contour, approx)){
			continue;
		}

		cv::Mat content(7, 7, CV_8U);
		extractContent(bw, content, approx);

		cv::Mat aruco(7, 7, CV_8U);
		cv::threshold(content, aruco, 160, 255, cv::THRESH_BINARY);

		ArucoValidator validator(aruco);
		if(!validator.validate()){
			continue;
		}

		Marker marker{ .id = static_cast<uint16_t>(validator.getID()) };

		for(uint8_t i = 0; i < 4; ++i){
			marker.projected[i] = {
					(float) approx[i].x / scale,
					(float) approx[i].y / scale
			};
		}

		for(uint8_t i = 0; i < validator.getRotation(); ++i){
			std::rotate(marker.projected.data(), marker.projected.data() + 1, marker.projected.data() + 4);
		}

		driveInfo.markerInfo.markers.emplace_back(marker);
	}

	std::sort(driveInfo.markerInfo.markers.begin(), driveInfo.markerInfo.markers.end(), [this](const Marker& m1, const Marker& m2) -> bool{
		return this->contourEval(m1.projected) > this->contourEval(m2.projected);
	});

	if(!driveInfo.markerInfo.markers.empty()){
		driveInfo.markerInfo.action = ArucoValidator::getAction(driveInfo.markerInfo.markers.front().id);
	}

	return true;
}

bool MarkerScanner::contourValid(const std::vector<cv::Point>& contour, std::vector<cv::Point>& approx){
	if(contour.size() < 4){
		return false;
	}

	cv::approxPolyDP(contour, approx, cv::arcLength(contour, true) * maxError, true);
	if(approx.size() != 4){
		return false;
	}

	if(cv::contourArea(approx, false) < minArea){
		return false;
	}

	if(!cv::isContourConvex(approx)){
		return false;
	}

	return true;
}

void MarkerScanner::extractContent(const cv::Mat& image, cv::Mat out, const std::vector<cv::Point>& contour){
	const std::array<cv::Point2f, 4> points = {
			cv::Point2f{ -0.5f, -0.5f },
			cv::Point2f{ -0.5f, (float) out.rows - 0.5f },
			cv::Point2f{ (float) out.cols - 0.5f, (float) out.rows - 0.5f },
			cv::Point2f{ (float) out.cols - 0.5f, -0.5f },
	};
	const std::array<cv::Point2f, 4> contourFloat = {
			cv::Point2f{ (float) contour[0].x, (float) contour[0].y },
			cv::Point2f{ (float) contour[1].x, (float) contour[1].y },
			cv::Point2f{ (float) contour[2].x, (float) contour[2].y },
			cv::Point2f{ (float) contour[3].x, (float) contour[3].y }
	};

	cv::Mat transform = cv::getPerspectiveTransform(contourFloat, points);

	warpPerspective(image, out, transform, out.size(), cv::INTER_CUBIC, cv::BORDER_CONSTANT, 255);
}

float MarkerScanner::contourEval(const std::array<std::pair<int16_t, int16_t>, 4>& contour) const{
	const glm::vec2 center = { width / 2, height / 2 };

	float area = 0.0f;
	area += 0.5f * (contour[1].first - contour[0].first) * (contour[1].second + contour[0].second);
	area += 0.5f * (contour[2].first - contour[1].first) * (contour[2].second + contour[1].second);
	area += 0.5f * (contour[3].first - contour[2].first) * (contour[3].second + contour[2].second);
	area += 0.5f * (contour[0].first - contour[3].first) * (contour[0].second + contour[3].second);

	glm::vec2 contourCenter{ 0.0f, 0.0f };
	for(uint8_t i = 0; i < 4; ++i){
		contourCenter.x += contour[i].first;
		contourCenter.y += contour[i].second;
	}

	contourCenter.x /= 4;
	contourCenter.y /= 4;

	return area / glm::distance(center, contourCenter);
}
