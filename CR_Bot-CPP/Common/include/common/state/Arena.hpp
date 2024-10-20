#pragma once

#include <vector>
#include <cpp-dump.hpp>
#include "Unit.hpp"

namespace state {
    struct Arena {
        std::vector<Unit> units;
    };
}
CPP_DUMP_DEFINE_EXPORT_OBJECT(state::Arena, units);
