#pragma once

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/dll.hpp>
#include <toml++/toml.hpp>
#include <spdlog/spdlog.h>
#include <eigen3/Eigen/Core>
#include <opencv2/opencv.hpp>

#include "common/card/CardHand.hpp"

namespace card {
	class Cropper {
	public:
		Eigen::Vector2i card_top_left;
		Eigen::Vector2i card_size;
		Eigen::Vector2i inner_card_size;
		Eigen::Vector2i inner_card_coords;
		int right_padding;

		Eigen::Vector2i next_card_top_left;
		Eigen::Vector2i next_card_size;
		int next_card_final_crop_height;

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

			this->inner_card_size = {
				battle_toml["card"]["inner_card_size"]["width"].value<int>().value(),
				battle_toml["card"]["inner_card_size"]["height"].value<int>().value()
			};

			this->inner_card_coords = {
				battle_toml["card"]["inner_card_coords"]["x"].value<int>().value(),
				battle_toml["card"]["inner_card_coords"]["y"].value<int>().value()
			};

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

		// This function crops the card from the screenshot
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
					this->card_top_left.x() + card_index * (this->inner_card_size.x() + this->right_padding),
					this->card_top_left.y(),
					this->inner_card_size.x(),
					this->inner_card_size.y()
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
	};
}
