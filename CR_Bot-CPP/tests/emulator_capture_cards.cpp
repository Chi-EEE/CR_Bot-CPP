#include <opencv2/opencv.hpp>
#include <spdlog/spdlog.h>

#include "Emulator.hpp"
#include "card/Cropper.hpp"

int main() {
	Emulator emulator("emulator-5556");
	cv::Mat screenshot = emulator.screenshot();
	card::Cropper cropper;

	cv::Mat first_card = cropper.crop_card(screenshot, card::CardHand::First);
	cv::Mat second_card = cropper.crop_card(screenshot, card::CardHand::Second);
	cv::Mat third_card = cropper.crop_card(screenshot, card::CardHand::Third);
	cv::Mat fourth_card = cropper.crop_card(screenshot, card::CardHand::Fourth);
	cv::Mat next_card = cropper.crop_card(screenshot, card::CardHand::Next);

	cv::imshow("First Card", first_card);
	cv::imshow("Second Card", second_card);
	cv::imshow("Third Card", third_card);
	cv::imshow("Fourth Card", fourth_card);
	cv::imshow("Next Card", next_card);

	cv::imwrite("first_card.png", first_card);
	cv::imwrite("second_card.png", second_card);
	cv::imwrite("third_card.png", third_card);
	cv::imwrite("fourth_card.png", fourth_card);

	cv::waitKey(0);

	cv::Mat first_card_min = cropper.crop_card_min(screenshot, card::CardHand::First);
	cv::Mat second_card_min = cropper.crop_card_min(screenshot, card::CardHand::Second);
	cv::Mat third_card_min = cropper.crop_card_min(screenshot, card::CardHand::Third);
	cv::Mat fourth_card_min = cropper.crop_card_min(screenshot, card::CardHand::Fourth);
	cv::Mat next_card_min = cropper.crop_card_min(screenshot, card::CardHand::Next);

	cv::imshow("First Card", first_card_min);
	cv::imshow("Second Card", second_card_min);
	cv::imshow("Third Card", third_card_min);
	cv::imshow("Fourth Card", fourth_card_min);
	cv::imshow("Next Card", next_card_min);

	cv::imwrite("first_card_min.png", first_card_min);
	cv::imwrite("second_card_min.png", second_card_min);
	cv::imwrite("third_card_min.png", third_card_min);
	cv::imwrite("fourth_card_min.png", fourth_card_min);

	cv::waitKey(0);
	return 0;
}
