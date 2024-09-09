#pragma once

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/asio.hpp>

#include <boost/dll.hpp>
#include <boost/process.hpp>

static void run_adb_command(const std::string device_serial, const std::vector<std::string> commands) {
	boost::process::child c(fmt::format("adb -s {} {}", device_serial, fmt::join(commands, " ")));
	c.wait();
}

static std::string run_adb_command_with_output(const std::string device_serial, const std::vector<std::string> commands) {
	std::stringstream output;
	boost::process::ipstream pipe_stream;
	boost::process::child c(fmt::format("adb -s {} {}", device_serial, fmt::join(commands, " ")), boost::process::std_out > pipe_stream);

	std::string line;
	while (pipe_stream && std::getline(pipe_stream, line) && !line.empty())
		output << line << std::endl;

	c.wait();
	return output.str();
}

static void click(const std::string device_serial, int x, int y) {
	run_adb_command(device_serial, { "shell", "input", "tap", std::to_string(x), std::to_string(y) });
}

static void start_game(const std::string device_serial) {
	run_adb_command(device_serial, { "shell", "am", "start", "-n", "com.supercell.clashroyale/com.supercell.titan.GameApp" });
}

static void stop_game(const std::string device_serial) {
	run_adb_command(device_serial, { "shell", "am", "force-stop", "com.supercell.clashroyale" });
}

static std::pair<int, int> get_size(const std::string device_serial) {
	std::string size_str = run_adb_command_with_output(device_serial, { "shell", "wm", "size" });
	size_str = size_str.substr(std::string("Physical size: ").size(), size_str.size());
	int width = std::stoi(size_str.substr(0, size_str.find("x")));
	int height = std::stoi(size_str.substr(size_str.find("x") + 1, size_str.size()));
	return std::make_pair(width, height);
}