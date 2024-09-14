#pragma once

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/dll.hpp>
#include <toml++/toml.hpp>
#include <spdlog/spdlog.h>
#include <eigen3/Eigen/Core>
#include <opencv2/opencv.hpp>

#include "Card.hpp"

namespace card {
	class Cropper {
	public:
		Cropper() {
			auto battle_constants_path = boost::dll::program_location().parent_path() / "constants" / "battle.toml";
			auto battle_toml = toml::parse_file(battle_constants_path.string());

			this->top_left = {
				battle_toml["card"]["top_left"]["x"].value<int>().value(),
				battle_toml["card"]["top_left"]["y"].value<int>().value()
			};

			this->size = {
				battle_toml["card"]["size"]["width"].value<int>().value(),
				battle_toml["card"]["size"]["height"].value<int>().value()
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
		}

		cv::Mat crop_card(cv::Mat screenshot, Card card) {
			cv::Rect roi;
			cv::Mat cropped;

			switch (card) {
			case Card::First:
			case Card::Second:
			case Card::Third:
			case Card::Fourth:
			{
				unsigned int card_index = static_cast<unsigned int>(card) - static_cast<unsigned int>(Card::First);
				roi = cv::Rect(
					this->top_left.x() + card_index * (this->size.x() + this->right_padding),
					this->top_left.y(),
					this->size.x(),
					this->size.y()
				);
				cropped = screenshot(roi);
				return cropped;
			}
			case Card::Next:
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

	private:
		Eigen::Vector2i top_left;
		Eigen::Vector2i size;
		int right_padding;

		Eigen::Vector2i next_card_top_left;
		Eigen::Vector2i next_card_size;
	};
}
