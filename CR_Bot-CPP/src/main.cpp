#include <iostream>

#include <boost/dll.hpp>
#include <boost/process.hpp>
#include <wels/codec_api.h>
#include <toml++/toml.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <tobiaslocker_base64/base64.hpp>

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

static void record(const std::string device_serial) {
    std::string command = R"(#!/bin/bash
while true; do
    screenrecord --output-format=h264 \
    --time-limit "179" \
    --size "720x1280" \
    --bit-rate "5M" -
done)";

	std::vector<std::string> commands = { "shell", "echo", command + "\n", "|", "base64", "-d", "|", "sh"};

    std::string command_str = fmt::format("adb -s {} {}", device_serial, fmt::join(commands, " "));
	std::cout << command_str << std::endl;
    boost::process::ipstream pipe_stream;
    boost::process::child c(command_str, boost::process::std_out > pipe_stream);

    std::string line;
    while (pipe_stream && std::getline(pipe_stream, line)/* && !line.empty()*/) {
        std::cout << line << std::endl;
    }

    c.wait();
}

int main()
{
    auto config_path = boost::dll::program_location().parent_path() / "config.toml";
    auto config_toml = toml::parse_file(config_path.string());

    auto maybe_ip = config_toml["adb"]["ip"].value<std::string>();
    if (!maybe_ip) {
		std::cerr << "No IP provided" << std::endl;
        return 1;
    }
    auto maybe_device_serial = config_toml["adb"]["device_serial"].value<std::string>();
    if (!maybe_device_serial) {
		std::cerr << "No device serial provided" << std::endl;
        return 1;
    }
    const std::string ip = maybe_ip.value_or("");
    const std::string device_serial = maybe_device_serial.value_or("");

    //get_size(device_serial);
    record(device_serial);

    std::cout << "Done";
    return 0;
}