#pragma once

#include <cpp-dump.hpp>

namespace state {
	enum Team {
		RED,
		BLUE,
	};
}
CPP_DUMP_DEFINE_EXPORT_ENUM(state::Team, state::Team::RED, state::Team::BLUE);
