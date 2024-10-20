#include <boost/dll.hpp>
#include <spdlog/spdlog.h>

#include "common/CR_Bot.hpp"

using namespace common;

int main()
{
	auto config_path = boost::dll::program_location().parent_path() / "config.toml";
	auto config_toml = toml::parse_file(config_path.string());

	auto maybe_ip = config_toml["adb"]["ip"].value<std::string>();
	if (!maybe_ip) {
		throw std::runtime_error("No IP provided");
	}

	auto maybe_device_serial = config_toml["adb"]["device_serial"].value<std::string>();
	if (!maybe_device_serial) {
		throw std::runtime_error("No device serial provided");
	}

	const std::string ip = maybe_ip.value_or("");
	const std::string device_serial = maybe_device_serial.value_or("");

	CR_Bot bot = CR_Bot(device_serial);
	std::chrono::steady_clock::time_point last_run_time = std::chrono::steady_clock::now();
	while (true) {
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_run_time);
		if (elapsed.count() < 10) {
			continue;
		}
		last_run_time = now;
		bot.step([&](const state::GameState& game_state) {
			return std::nullopt;
			}
		);
	}

	std::cout << "Done";
	return 0;
}