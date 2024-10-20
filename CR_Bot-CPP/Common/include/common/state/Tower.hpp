#pragma once

#include <cpp-dump.hpp>

namespace state
{
	struct Tower {
		int health = 0;
	};
}
CPP_DUMP_DEFINE_EXPORT_OBJECT(state::Tower, health);
