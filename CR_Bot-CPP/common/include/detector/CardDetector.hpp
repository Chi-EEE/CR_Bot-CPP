#pragma once

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/dll.hpp>
#include <toml++/toml.hpp>
#include <spdlog/spdlog.h>
#include <eigen3/Eigen/Core>

class CardDetector {
	CardDetector() {
		auto battle_constants_path = boost::dll::program_location().parent_path() / "constants" / "battle.toml";
		auto battle_toml = toml::parse_file(battle_constants_path.string());

		Eigen::Vector2i top_left = {
			battle_toml["card"]["top_left"]["x"].value<int>().value(),
			battle_toml["card"]["top_left"]["y"].value<int>().value()
		};

		Eigen::Vector2i size = {
			battle_toml["card"]["size"]["width"].value<int>().value(),
			battle_toml["card"]["size"]["height"].value<int>().value()
		};

		const int right_padding = battle_toml["card"]["right_padding"].value<int>().value();

	}
};