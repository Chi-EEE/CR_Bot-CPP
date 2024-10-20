#pragma once

#include <cpp-dump.hpp>
#include "Team.hpp"

namespace state {
	struct Unit {
		Team team;
		int health = 0;
	};
}
CPP_DUMP_DEFINE_EXPORT_OBJECT(state::Unit, team, health);
