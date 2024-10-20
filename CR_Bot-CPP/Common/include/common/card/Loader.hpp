#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include <boost/dll.hpp>
#include <toml++/toml.hpp>
#include <spdlog/spdlog.h>
#include <opencv2/opencv.hpp>
#include <magic_enum.hpp>

#include "common/card/BaseCard.hpp"

namespace card {
	class Loader {
	public:
		Loader() {
			auto card_constants_path = boost::dll::program_location().parent_path() / "constants" / "card.toml";
			auto card_toml = toml::parse_file(card_constants_path.string());
			auto snakeToUpperSnakeCase = [](std::string_view snake_case) -> std::string {
				std::stringstream result;
				bool capitalizeNext = true;

				for (char ch : snake_case) {
					if (ch == '_') {
						capitalizeNext = true;
						result << ch;
					}
					else {
						if (capitalizeNext) {
							result << static_cast<char>(toupper(ch));
							capitalizeNext = false;
						}
						else {
							result << ch;
						}
					}
				}

				return result.str();
				};
			for (const auto& [card_name, card_table_node] : *card_toml["card"].as_table()) {
				const auto& card_table = *card_table_node.as_table();
				BaseCard card;
				std::string type_str = snakeToUpperSnakeCase(card_name.str());
				card.type = magic_enum::enum_cast<Card>(type_str).value();
				card.path = card_table["path"].value_or("");
				card.id = static_cast<uint8_t>(card_table["id"].value<int64_t>().value());
				card.elixir = static_cast<uint8_t>(card_table["elixir"].value<int64_t>().value());
				if (auto evo_path = card_table["evo_path"].value<std::string>()) {
					card.evo_path = evo_path.value();
				}
				if (card_table["china"].value<bool>()) {
					card.china = true;
				}
				this->cards.insert({ std::string(card_name.str()), card });
			}
		}

		BaseCard get_card(std::string card_name) {
			return this->cards[card_name];
		}

		BaseCard get_card(Card card) {
			std::string card_name = std::string(magic_enum::enum_name(card));
			std::transform(card_name.begin(), card_name.end(), card_name.begin(),
				[](unsigned char c) { return std::tolower(c); });
			return this->cards[card_name];
		}

		std::map<std::string, BaseCard>& get_cards() {
			return this->cards;
		}
	private:
		std::map<std::string, BaseCard> cards;
	};
}
