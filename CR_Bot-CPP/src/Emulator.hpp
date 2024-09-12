#pragma once

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/asio.hpp>

#include <boost/dll.hpp>
#include <boost/process.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

class Emulator {
public:
	void click(int x, int y) {
		this->run_adb_command({ "shell", "input", "tap", std::to_string(x), std::to_string(y) });
	}

	void start_game() {
		this->run_adb_command({ "shell", "am", "start", "-n", "com.supercell.clashroyale/com.supercell.titan.GameApp" });
	}

	void stop_game() {
		this->run_adb_command({ "shell", "am", "force-stop", "com.supercell.clashroyale" });
	}

	[[nodiscard]]
	std::pair<int, int> get_size(const std::string device_serial) {
		std::string size_str = this->run_adb_command_with_output(device_serial, { "shell", "wm", "size" });
		size_str = size_str.substr(std::string("Physical size: ").size(), size_str.size());
		int width = std::stoi(size_str.substr(0, size_str.find("x")));
		int height = std::stoi(size_str.substr(size_str.find("x") + 1, size_str.size()));
		return std::make_pair(width, height);
	}

	void run_adb_command(const std::vector<std::string> commands) {
		boost::process::child c(fmt::format("adb -s {} {}", this->device_serial, fmt::join(commands, " ")));
		c.wait();
	}

	[[nodiscard]]
	std::string run_adb_command_with_output(const std::string device_serial, const std::vector<std::string> commands) {
		std::stringstream output;
		boost::process::ipstream pipe_stream;
		boost::process::child c(fmt::format("adb -s {} {}", device_serial, fmt::join(commands, " ")), boost::process::std_out > pipe_stream);

		std::string line;
		while (pipe_stream && std::getline(pipe_stream, line) && !line.empty())
			output << line << std::endl;

		c.wait();
		return output.str();
	}
private:
	const std::string device_serial;
};