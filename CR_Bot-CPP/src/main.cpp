#include <iostream>

#include <boost/dll.hpp>
#include <boost/process.hpp>
#include <fmt/format.h>
#include <wels/codec_api.h>
#include <toml++/toml.hpp>

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
    std::string ip = maybe_ip.value_or("");
    std::string device_serial = maybe_device_serial.value_or("");

    boost::process::ipstream pipe_stream;
    boost::process::child c(fmt::format("adb -s {} shell wm size", device_serial), boost::process::std_out > pipe_stream);

    std::string line;

    while (pipe_stream && std::getline(pipe_stream, line) && !line.empty())
        std::cerr << line << std::endl;

    c.wait();

    std::cout << "Done";
    return 0;
}