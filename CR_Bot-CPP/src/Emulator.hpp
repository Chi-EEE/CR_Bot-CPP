#pragma once

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/asio.hpp>

#include <boost/dll.hpp>
#include <boost/process.hpp>

#include <spdlog/spdlog.h>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <tobiaslocker_base64/base64.hpp>

#include "video/CodecContext.hpp"

//static std::string screen_record_bash = R"(#!/bin/bash
//while true; do
//    screenrecord --output-format=h264 \
//    --time-limit "179" \
//    --size "720x1280" \
//    --bit-rate "5M" -
//done
//)";

class Emulator {
public:
	Emulator(const std::string device_serial): device_serial(device_serial) {
	}

	~Emulator() {

	}

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
		std::string size_str = this->run_adb_command_with_output(this->device_serial, { "shell", "wm", "size" });
		size_str = size_str.substr(std::string("Physical size: ").size(), size_str.size());
		int width = std::stoi(size_str.substr(0, size_str.find("x")));
		int height = std::stoi(size_str.substr(size_str.find("x") + 1, size_str.size()));
		return std::make_pair(width, height);
	}

	void record() {
		std::unique_ptr<CodecContext> codec_context = std::make_unique<CodecContext>();

		//std::string record_bash_base64 = base64::to_base64(screen_record_bash);
		std::vector<std::string> commands = { "exec-out", "screenrecord", "--output-format=h264", "-" };

		std::string command_str = fmt::format("adb -s {} {}", this->device_serial, fmt::join(commands, " "));
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
						spdlog::error("Error: {}", ec.message());
					}
				}
			);
			};

		// Start the first read
		do_read();

		ioc.run();
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