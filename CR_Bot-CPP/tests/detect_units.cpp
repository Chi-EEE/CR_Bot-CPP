#include <iostream>
#include <vector>
#include <algorithm>

#include <boost/dll.hpp>
#include <opencv2/opencv.hpp>

#include "card/Card.hpp"
#include "card/Loader.hpp"
#include "card/Cropper.hpp"
#include "card/Detector.hpp"

int main() {
	auto program_dir = boost::dll::program_location().parent_path();
	auto screenshots_path = program_dir / "assets" / "screenshots";

	card::Loader loader;
	card::Cropper cropper;
	const std::map<std::string, card::BaseCard>& cards_map = loader.get_cards();

	std::vector<card::BaseCard> cards;
	for (const auto& c : cards_map) { cards.push_back(c.second); }

	card::Detector detector(cropper, cards);
	cv::Mat image = cv::imread((screenshots_path / "2.png").string(), cv::IMREAD_COLOR);

	cv::Mat first_card = cropper.crop_card(image, card::CardHand::First);
	cv::Mat second_card = cropper.crop_card(image, card::CardHand::Second);
	cv::Mat third_card = cropper.crop_card(image, card::CardHand::Third);
	cv::Mat fourth_card = cropper.crop_card(image, card::CardHand::Fourth);

	auto detected_cards = detector.run({
		first_card,
		second_card,
		third_card,
		fourth_card,
		});

	for (const auto& card : detected_cards) {
		cpp_dump(card.type);
	}

	return 0;
}
