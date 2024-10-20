#pragma once

#include <string>
#include <cpp-dump.hpp>
#include "Position.hpp"

namespace state {
	struct ActionCard {
		std::string card_name;
		Position tile_position;
	};
}
CPP_DUMP_DEFINE_EXPORT_OBJECT(state::ActionCard, card_name, tile_position);
