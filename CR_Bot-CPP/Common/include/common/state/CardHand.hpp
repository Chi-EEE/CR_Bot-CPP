#pragma once

#include <string>
#include <cpp-dump.hpp>
#include "Card.hpp"

namespace state {
	struct CardHand {
		Card card_1;
		Card card_2;
		Card card_3;
		Card card_4;
	};
}
CPP_DUMP_DEFINE_EXPORT_OBJECT(state::CardHand, card_1, card_2, card_3, card_4);
