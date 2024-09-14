#pragma once

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/dll.hpp>
#include <toml++/toml.hpp>
#include <spdlog/spdlog.h>
#include <eigen3/Eigen/Core>
#include <opencv2/opencv.hpp>

#include "CardHand.hpp"

namespace card {
	class Cropper {
	public:
		Cropper() {
			auto battle_constants_path = boost::dll::program_location().parent_path() / "constants" / "battle.toml";
			auto battle_toml = toml::parse_file(battle_constants_path.string());

			this->card_top_left = {
				battle_toml["card"]["top_left"]["x"].value<int>().value(),
				battle_toml["card"]["top_left"]["y"].value<int>().value()
			};

			this->card_size = {
				battle_toml["card"]["size"]["width"].value<int>().value(),
				battle_toml["card"]["size"]["height"].value<int>().value()
			};

			this->card_final_crop_height = battle_toml["card"]["final_crop_height"].value<int>().value();

			this->right_padding = battle_toml["card"]["right_padding"].value<int>().value();

			this->next_card_top_left = {
				battle_toml["next_card"]["top_left"]["x"].value<int>().value(),
				battle_toml["next_card"]["top_left"]["y"].value<int>().value()
			};

			this->next_card_size = {
				battle_toml["next_card"]["size"]["width"].value<int>().value(),
				battle_toml["next_card"]["size"]["height"].value<int>().value()
			};

			this->next_card_final_crop_height = battle_toml["next_card"]["final_crop_height"].value<int>().value();
		}

		// This function crops the card from the screenshot (Used for testing)
		cv::Mat crop_card(cv::Mat screenshot, CardHand card) {
			cv::Rect roi;
			cv::Mat cropped;

			switch (card) {
			case CardHand::First:
			case CardHand::Second:
			case CardHand::Third:
			case CardHand::Fourth:
			{
				unsigned int card_index = static_cast<unsigned int>(card) - static_cast<unsigned int>(CardHand::First);
				roi = cv::Rect(
					this->card_top_left.x() + card_index * (this->card_size.x() + this->right_padding),
					this->card_top_left.y(),
					this->card_size.x(),
					this->card_size.y()
				);
				cropped = screenshot(roi);
				return cropped;
			}
			case CardHand::Next:
				roi = cv::Rect(
					this->next_card_top_left.x(),
					this->next_card_top_left.y(),
					this->next_card_size.x(),
					this->next_card_size.y()
				);
				cropped = screenshot(roi);
				return cropped;
			}
		}

		// This function is used to crop the card with the minimum height
		// This is useful as we are going to use the cropped image to compare with the stored images without the elixir count
		cv::Mat crop_card_min(cv::Mat screenshot, CardHand card) {
			cv::Rect roi;
			cv::Mat cropped;

			switch (card) {
			case CardHand::First:
			case CardHand::Second:
			case CardHand::Third:
			case CardHand::Fourth:
			{
				unsigned int card_index = static_cast<unsigned int>(card) - static_cast<unsigned int>(CardHand::First);
				roi = cv::Rect(
					this->card_top_left.x() + card_index * (this->card_size.x() + this->right_padding),
					this->card_top_left.y(),
					this->card_size.x(),
					this->card_final_crop_height
				);
				cropped = screenshot(roi);
				return cropped;
			}
			case CardHand::Next:
				roi = cv::Rect(
					this->next_card_top_left.x(),
					this->next_card_top_left.y(),
					this->next_card_size.x(),
					this->next_card_final_crop_height
				);
				cropped = screenshot(roi);
				return cropped;
			}
		}

	private:
		Eigen::Vector2i card_top_left;
		Eigen::Vector2i card_size;
		int card_final_crop_height;
		int right_padding;

		Eigen::Vector2i next_card_top_left;
		Eigen::Vector2i next_card_size;
		int next_card_final_crop_height;
	};
}
