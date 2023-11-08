#ifndef PERSE_ROVER_MARKERSCANNER_H
#define PERSE_ROVER_MARKERSCANNER_H

#include <cstdint>
#include <DriveInfo.h>

#undef EPS

#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>

class MarkerScanner {
public:
	MarkerScanner(uint8_t frameWidth, uint8_t frameHeight);
	
	bool process(const uint8_t* rawFrame, DriveInfo& driveInfo);

private:
	uint8_t width = 0;
	uint8_t height = 0;

	static constexpr float accuracy = 0.2f;
	static constexpr float scaleMin = 0.5f;
	static constexpr float scaleMax = 1.0f;
	static constexpr float boxMin = 15.0f;
	static constexpr float boxMax = 23.0f;
	static constexpr float limitCosSqr = 0.3f;
	static constexpr float maxError = 0.025f;
	static constexpr int minArea = 7 * 7;

	int8_t box;
	float scale;

	cv::Mat small;
	cv::Mat bw;

	std::unique_ptr<uint8_t> smallData;
	std::unique_ptr<uint8_t> bwData;

private:
	static bool contourValid(const std::vector<cv::Point>& contour, std::vector<cv::Point>& approx);
	static void extractContent(const cv::Mat& image, cv::Mat out, const std::vector<cv::Point>& contour);
	float contourEval(const std::array<std::pair<int16_t, int16_t>, 4>& contour) const;
};

#endif //PERSE_ROVER_MARKERSCANNER_H