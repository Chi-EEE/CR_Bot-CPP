#include <iostream>

#include <boost/dll.hpp>

#include <sol/sol.hpp>
#include <sol/types.hpp>
#include <magic_enum.hpp>

#include <cpp-dump.hpp>

#include "common/CR_Bot.hpp"

#include "common/state/GameState.hpp"
#include "common/state/ActionCard.hpp"

namespace lua {
	class Lua_Bot : public common::CR_Bot {
	public:
		Lua_Bot(std::string device_serial) : common::CR_Bot(device_serial) {
		}

		void lua_step(sol::this_state state, sol::function callback) {
			this->step([&](const state::GameState& game_state) {
				return std::nullopt;
				}
			);
		}

	private:
	};
}

int main() {
	auto main_lua = boost::dll::program_location().parent_path() / "main.lua";

	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::coroutine);
	lua.set_function("tick", []() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}
	);

	auto bot = lua["cr"].get_or_create<sol::table>();
	bot.new_usertype<lua::Lua_Bot>("Bot",
		"step", &lua::Lua_Bot::lua_step,
		"start_record", &lua::Lua_Bot::start_record
	);

	lua.script_file(main_lua.string());

	return 0;
}