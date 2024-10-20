#pragma once

#include <cpp-dump.hpp>
#include "CardHand.hpp"
#include "Card.hpp"
#include "Player.hpp"
#include "Arena.hpp"

namespace state {
	struct GameState {
		CardHand card_hand;
		Card next_card;
		Player current_player;
		Player opponent_player;
		Arena arena;
	};
}
CPP_DUMP_DEFINE_EXPORT_OBJECT(state::GameState, card_hand, next_card, current_player, opponent_player, arena);
