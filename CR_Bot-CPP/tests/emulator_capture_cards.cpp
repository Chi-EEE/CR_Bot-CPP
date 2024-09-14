#include <opencv2/opencv.hpp>
#include <spdlog/spdlog.h>

#include "Emulator.hpp"
#include "card/Cropper.hpp"

int main() {
	Emulator emulator("emulator-5554");
	cv::Mat screenshot = emulator.screenshot();
	card::Cropper cropper;

	cv::Mat first_card = cropper.crop_card(screenshot, card::Card::First);
	cv::imshow("First Card", first_card);

	cv::Mat second_card = cropper.crop_card(screenshot, card::Card::Second);
	cv::imshow("Second Card", second_card);

	cv::Mat third_card = cropper.crop_card(screenshot, card::Card::Third);
	cv::imshow("Third Card", third_card);

	cv::Mat fourth_card = cropper.crop_card(screenshot, card::Card::Fourth);
	cv::imshow("Fourth Card", fourth_card);

	cv::Mat next_card = cropper.crop_card(screenshot, card::Card::Next);
	cv::imshow("Next Card", next_card);

	cv::waitKey(0);
	return 0;
}
