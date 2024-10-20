#pragma once

#include <string>
#include <cpp-dump.hpp>

namespace state {
	struct Card {
		std::string name;
		int elixir_cost = 0;
	};
}
CPP_DUMP_DEFINE_EXPORT_OBJECT(state::Card, name, elixir_cost);
