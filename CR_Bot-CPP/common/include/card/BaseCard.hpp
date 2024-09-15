#pragma once

#include <string>
#include <optional>

#include <cpp-dump.hpp>

#include "card/Card.hpp"

namespace card
{
    struct BaseCard
    {
        Card type = Card::Blank;
        std::string path = "assets/cards/blank.png";
        int id = 0;
        int elixir = 0;
        std::optional<std::string> evo_path;
        bool china = false;
    };
}
CPP_DUMP_DEFINE_EXPORT_OBJECT(card::BaseCard, type, id, elixir, path, evo_path);
