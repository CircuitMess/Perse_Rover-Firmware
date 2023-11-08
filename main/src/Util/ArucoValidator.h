#ifndef PERSE_ROVER_ARUCOVALIDATOR_H
#define PERSE_ROVER_ARUCOVALIDATOR_H

#include <opencv2/core/types.hpp>
#include <map>
#include <MarkerInfo.h>

class ArucoValidator {
public:
	explicit ArucoValidator(const cv::Mat& aruco);

	bool validate();
	int16_t getID() const;
	uint8_t getRotation() const;
	MarkerAction getAction();
	static MarkerAction getAction(int16_t id);

private:
	static const std::map<int16_t, MarkerAction> IdToAction;
	uint8_t cells[7][7];
	int16_t id = -1;
	uint8_t rotation = 0;

private:
	int hammingDistance();
	void rotate();
	void calcID();
};

#endif //PERSE_ROVER_ARUCOVALIDATOR_H