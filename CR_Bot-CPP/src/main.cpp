#include <boost/dll.hpp>
#include <toml++/toml.hpp>

#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include "Emulator.hpp"

int main()
{
	auto config_path = boost::dll::program_location().parent_path() / "config.toml";
	auto config_toml = toml::parse_file(config_path.string());

	auto maybe_ip = config_toml["adb"]["ip"].value<std::string>();
	if (!maybe_ip) {
		spdlog::error("No IP provided");
		return 1;
	}
	auto maybe_device_serial = config_toml["adb"]["device_serial"].value<std::string>();
	if (!maybe_device_serial) {
		spdlog::error("No device serial provided");
		return 1;
	}
	const std::string ip = maybe_ip.value_or("");
	const std::string device_serial = maybe_device_serial.value_or("");

	Emulator emulator = Emulator(device_serial);
	emulator.record();

	std::cout << "Done";
	return 0;
}