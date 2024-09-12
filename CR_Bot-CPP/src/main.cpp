#include <iostream>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/asio.hpp>

#include <boost/dll.hpp>
#include <boost/process.hpp>

#include <toml++/toml.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <tobiaslocker_base64/base64.hpp>

#include <opencv2/opencv.hpp>
#include <png.h>

#include "video/CodecContext.hpp"

#include "video/VideoFrame.hpp"
#include "video/Packet.hpp"

#include "video/VideoReformatter.hpp"
#include "video/VideoFormat.hpp"

//#include "Emulator.hpp"

static std::string screen_record_bash = R"(#!/bin/bash
while true; do
    screenrecord --output-format=h264 \
    --time-limit "179" \
    --size "720x1280" \
    --bit-rate "5M" -
done
)";


static std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}


static void record(const std::string device_serial) {
	//av_log_set_level(AV_LOG_QUIET);
	std::unique_ptr<CodecContext> codec_context = std::make_unique<CodecContext>();

	std::string record_bash_base64 = base64::to_base64(screen_record_bash);
	std::vector<std::string> commands = { "exec-out", "screenrecord", "--output-format=h264", "-" };

	std::string command_str = fmt::format("adb -s {} {}", device_serial, fmt::join(commands, " "));
	std::cout << command_str << std::endl;

	boost::asio::io_context ioc;
	boost::process::async_pipe pipe(ioc);
	boost::process::child c(command_str, boost::process::std_out > pipe, ioc);

	int frame_number = 0;

	VideoReformatter reformatter;
	std::array<uint8_t, 2048> buffer;
	std::function<void()> do_read;
	do_read = [&]() {
		boost::asio::async_read(pipe, boost::asio::buffer(buffer),
			[&](boost::system::error_code ec, std::size_t length) {
				if (!ec) {
					// Process the received line
					std::vector<std::unique_ptr<Packet>> packets = codec_context->parse(buffer.data(), buffer.size());
					if (packets.empty()) {
						do_read();
						return;
					}

					std::unique_ptr<Packet>& packet = packets.back();

					std::vector<std::unique_ptr<VideoFrame>> frames = codec_context->decode(packet);

					for (std::unique_ptr<VideoFrame>& frame : frames) {
						std::unique_ptr<VideoFrame> reformatted_frame = reformatter.reformat(frame.get(), 720, 1280, AV_PIX_FMT_BGR24);
						if (!reformatted_frame) {
							continue;
						}

						cv::Mat image = reformatted_frame->to_image();
						cv::imwrite(fmt::format("frame_{}.png", frame_number), image);

						frame_number++;
					}

					do_read();  // Initiate the next asynchronous read
				}
				else {
					std::cout << "Error: " << ec.message() << std::endl;
				}
			}
		);
		};

	// Start the first read
	do_read();

	ioc.run();
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
	record(device_serial);

	std::cout << "Done";
	return 0;
}