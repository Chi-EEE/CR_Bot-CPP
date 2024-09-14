#include <opencv2/opencv.hpp>
#include <spdlog/spdlog.h>

#include "Emulator.hpp"
#include "card/Cropper.hpp"
#include "card/Detector.hpp"

int main() {
	Emulator emulator("emulator-5554");
	cv::Mat screenshot = emulator.screenshot();
	card::Cropper cropper;
	card::Detector detector;

	cv::Mat first_card = cropper.crop_card_min(screenshot, card::CardHand::First);
	cv::Mat second_card = cropper.crop_card_min(screenshot, card::CardHand::Second);
	cv::Mat third_card = cropper.crop_card_min(screenshot, card::CardHand::Third);
	cv::Mat fourth_card = cropper.crop_card_min(screenshot, card::CardHand::Fourth);
	cv::Mat next_card = cropper.crop_card_min(screenshot, card::CardHand::Next);

	cv::waitKey(0);
	return 0;
}
