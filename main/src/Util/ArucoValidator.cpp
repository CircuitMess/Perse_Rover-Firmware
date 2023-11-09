#include "ArucoValidator.h"

#undef EPS

#include <opencv2/imgproc.hpp>

ArucoValidator::ArucoValidator(const cv::Mat& aruco){
	for(uint8_t i = 0; i < 7 * 7; ++i){
		cells[i / 7][i % 7] = (aruco.data[i] == 255);
	}
}

bool ArucoValidator::validate(){
	int black = 0, white = 0;
	for(uint8_t x = 0; x < 7; ++x){
		for(uint8_t y = 0; y < 7; ++y){
			black += cells[x][y] == 0;
			white += cells[x][y] == 1;
		}
	}

	if(black == 7 * 7 || white == 7 * 7){
		return false;
	}

	uint8_t bad = 0;
	for(uint8_t i = 0; i < 7; ++i){
		if(cells[i][0] != 1 || cells[i][6] != 1 || cells[0][i] != 1 || cells[6][i] != 1){
			bad++;
			if(bad > 3){
				return false;
			}
		}
	}

	for(uint8_t i = 0; i < 7; ++i){
		for(uint8_t j = 0; j < 7; ++j){
			cells[i][j] = !cells[i][j];
		}
	}

	for(int j = 0; j < 4; j++){
		if(hammingDistance() == 0){
			calcID();
			return true;
		}

		rotate();
	}

	return false;
}

int16_t ArucoValidator::getID() const{
	return id;
}

uint8_t ArucoValidator::getRotation() const{
	return rotation;
}

int ArucoValidator::hammingDistance(){
	static constexpr uint8_t ids[4][5] = {
			{ 1, 0, 0, 0, 0 },
			{ 1, 0, 1, 1, 1 },
			{ 0, 1, 0, 0, 1 },
			{ 0, 1, 1, 1, 0 }
	};

	int dist = 0;
	int sum, minSum;

	for(uint8_t i = 1; i < 6; ++i){
		minSum = 99999;

		for(uint8_t j = 1; j < 5; ++j){
			sum = 0;

			for(uint8_t k = 1; k < 6; ++k){
				sum += cells[i][k] == ids[j - 1][k - 1] ? 0 : 1;
			}

			if(sum < minSum){
				minSum = sum;
			}
		}

		dist += minSum;
	}

	return dist;
}

void ArucoValidator::rotate(){
	int temp[7][7];
	uint8_t n = 7;

	for(uint8_t i = 0; i < n; ++i){
		for(uint8_t j = 0; j < n; ++j){
			temp[i][j] = cells[n - j - 1][i];
		}
	}

	for(uint8_t i = 0; i < n; ++i){
		for(uint8_t j = 0; j < n; ++j){
			cells[i][j] = temp[i][j];
		}
	}

	rotation++;
}

void ArucoValidator::calcID(){
	id = 0;

	for(uint8_t i = 1; i < 6; ++i){
		id <<= 1;
		id |= cells[i][2];
		id <<= 1;
		id |= cells[i][4];
	}
}

MarkerAction ArucoValidator::getAction(){
	return getAction(getID());
}

MarkerAction ArucoValidator::getAction(int16_t id){

}
