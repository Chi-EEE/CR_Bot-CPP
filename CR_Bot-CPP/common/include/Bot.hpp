#pragma once

#include <toml++/toml.hpp>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <tl/expected.hpp>

#include "Emulator.hpp"

class Bot {
	explicit Bot(std::string device_serial) : emulator(Emulator(device_serial)) {
	}

public:
	static tl::expected<Bot, std::string> create(std::string config_toml_path) {
		auto config_toml = toml::parse_file(config_toml_path);

		auto maybe_ip = config_toml["adb"]["ip"].value<std::string>();
		if (!maybe_ip) {
			throw tl::make_unexpected<std::string>("No IP provided");
		}
		auto maybe_device_serial = config_toml["adb"]["device_serial"].value<std::string>();
		if (!maybe_device_serial) {
			return tl::make_unexpected<std::string>("No device serial provided");
		}

		const std::string ip = maybe_ip.value_or("");
		const std::string device_serial = maybe_device_serial.value_or("");

		return Bot(device_serial);
	}

	void run() {
		std::thread record_thread(&Emulator::start_record, &this->emulator);
		while (true) {
			this->emulator.click(100, 100);
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

private:
	Emulator emulator;
};
