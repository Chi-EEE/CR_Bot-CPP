#include <opencv2/opencv.hpp>
#include <spdlog/spdlog.h>

#include "Emulator.hpp"

int main() {
	Emulator emulator("emulator-5554");
	cv::Mat screenshot = emulator.screenshot();
	cv::imshow("Screenshot", screenshot);
	cv::waitKey(0);
	return 0;
}
