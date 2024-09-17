#include <boost/dll.hpp>
#include <spdlog/spdlog.h>

#include "Bot.hpp"

int main()
{
	auto config_path = boost::dll::program_location().parent_path() / "config.toml";
	auto maybe_bot = Bot::create(config_path.string());
	if (!maybe_bot) {
		spdlog::error("{}", maybe_bot.error());
		return 1;
	}

	Bot& bot = maybe_bot.value();
	bot.run();

	std::cout << "Done";
	return 0;
}