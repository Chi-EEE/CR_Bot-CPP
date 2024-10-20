#pragma once

#include <cpp-dump.hpp>

namespace state {
	struct Position {
		int x = 0;
		int y = 0;
	};
}
CPP_DUMP_DEFINE_EXPORT_OBJECT(state::Position, x, y);
