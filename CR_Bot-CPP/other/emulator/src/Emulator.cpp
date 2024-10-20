#include "emulator/Emulator.h"

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/asio.hpp>

#include <boost/dll.hpp>
#include <boost/process.hpp>

#include <spdlog/spdlog.h>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <tobiaslocker_base64/base64.hpp>

#include "av/CodecContext.hpp"

namespace emulator {
	void Emulator::click(int x, int y) const {
		this->run_adb_command({ "shell", "input", "tap", std::to_string(x), std::to_string(y) });
	}

	void Emulator::start_app(const std::string& app) const {
		this->run_adb_command({ "shell", "am", "start", "-n", app });
	}

	void Emulator::stop_app(const std::string& app) const {
		this->run_adb_command({ "shell", "am", "force-stop", app });
	}

	[[nodiscard]]
	std::pair<int, int> Emulator::get_size(const std::string device_serial) const {
		std::string size_str = this->run_adb_command_with_output({ "shell", "wm", "size" });
		size_str = size_str.substr(std::string("Physical size: ").size(), size_str.size());
		int width = std::stoi(size_str.substr(0, size_str.find("x")));
		int height = std::stoi(size_str.substr(size_str.find("x") + 1, size_str.size()));
		return std::make_pair(width, height);
	}

	[[nodiscard]]
	cv::Mat Emulator::screenshot() const {
		std::string image_str = this->run_adb_command_with_output({ "exec-out", "screencap", "-p" });
		std::vector<uint8_t> image_data(image_str.begin(), image_str.end());
		cv::Mat image = cv::imdecode(std::move(image_data), cv::IMREAD_COLOR);
		return image;
	}

	void Emulator::start_record() const {
		std::unique_ptr<av::CodecContext> codec_context = std::make_unique<av::CodecContext>();

		std::vector<std::string> commands = { "exec-out", "screenrecord", "--output-format=h264", "--bit-rate", R"("5M")", "-" };

		std::string command_str = fmt::format("adb -s {} {}", this->device_serial, fmt::join(commands, " "));
		std::cout << command_str << std::endl;

		boost::asio::io_context ioc;
		boost::process::async_pipe pipe(ioc);
		boost::process::child c(command_str, boost::process::std_out > pipe, ioc);

		av::VideoReformatter reformatter;
		// The buffer '2048' is the only highest buffer size that works and is the quickest
		std::array<uint8_t, 2048> buffer;
		std::function<void()> do_read;
		do_read = [&]() {
			boost::asio::async_read(pipe, boost::asio::buffer(buffer),
				[&](boost::system::error_code ec, std::size_t length) {
					if (!ec) {
						// Process the received line
						std::vector<std::unique_ptr<av::Packet>> packets = codec_context->parse(buffer.data(), buffer.size());
						if (packets.empty()) {
							do_read();
							return;
						}

						std::unique_ptr<av::Packet>& packet = packets.back();

						std::vector<std::unique_ptr<av::VideoFrame>> frames = codec_context->decode(packet);

						for (std::unique_ptr<av::VideoFrame>& frame : frames) {
							std::unique_ptr<av::VideoFrame> reformatted_frame = reformatter.reformat(frame.get(), 720, 1280, AV_PIX_FMT_BGR24);
							if (!reformatted_frame) {
								continue;
							}

							cv::Mat image = reformatted_frame->to_image();
							this->last_frame = image;
						}

						// Initiate the next asynchronous read
						do_read();
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

	void Emulator::run_adb_command(const std::vector<std::string> commands) const {
		boost::process::child c(fmt::format("adb -s {} {}", this->device_serial, fmt::join(commands, " ")));
		c.wait();
	}

	// https://stackoverflow.com/a/67640372
	[[nodiscard]]
	std::string Emulator::run_adb_command_with_output(const std::vector<std::string> commands) const {
		boost::asio::io_context ioc;
		std::future<std::string> data;
		boost::process::child c(fmt::format("adb -s {} {}", this->device_serial, fmt::join(commands, " ")), boost::process::std_out > data, ioc);

		c.detach();

		ioc.run();

		return data.get();
	}
}
