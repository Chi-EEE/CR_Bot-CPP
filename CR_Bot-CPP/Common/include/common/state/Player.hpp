#pragma once

#include <string>
#include <cpp-dump.hpp>
#include "CardHand.hpp"
#include "Tower.hpp"

namespace state {
	struct Player {
		std::string name;
		int elixir = 0;
		CardHand card_hand;
		Tower left_tower;
		Tower right_tower;
	};
}
CPP_DUMP_DEFINE_EXPORT_OBJECT(state::Player, name, elixir, card_hand, left_tower, right_tower);
