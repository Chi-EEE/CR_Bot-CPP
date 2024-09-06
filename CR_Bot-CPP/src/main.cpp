#include <iostream>

#include <boost/dll.hpp>
#include <boost/process.hpp>
#include <wels/codec_api.h>
#include <toml++/toml.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <tobiaslocker_base64/base64.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

static unsigned char screen_record_bash[] = {
	#include "screen_record.bash.h"
};

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

static std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

static void record(const std::string device_serial) {
	av_log_set_level(AV_LOG_QUIET);

	const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec) {
		fprintf(stderr, "Codec not found\n");
		return;
	}

	AVCodecParserContext* parser = av_parser_init(codec->id);
	if (!parser) {
		fprintf(stderr, "Parser not found\n");
		return;
	}

	AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
	if (!codecCtx) {
		fprintf(stderr, "Could not allocate video codec context\n");
		return;
	}

	if (avcodec_open2(codecCtx, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		return;
	}

	std::vector<std::string> commands = { "shell", "echo", std::string(reinterpret_cast<char*>(screen_record_bash), sizeof(screen_record_bash)) + "\n", "|", "base64", "-d", "|", "sh" };

	std::string command_str = fmt::format("adb -s {} {}", device_serial, fmt::join(commands, " "));
	std::cout << command_str << std::endl;
	boost::process::ipstream pipe_stream;
	boost::process::child c(command_str, boost::process::std_out > pipe_stream);

	//int count = 0;
	std::string line;
	while (pipe_stream && std::getline(pipe_stream, line)/* && !line.empty()*/) {
		line = ReplaceAll(line, "\r\n", "\n");
		//count++;

		const uint8_t* data = reinterpret_cast<const uint8_t*>(line.data());
		int size = static_cast<int>(line.size());

		AVPacket packet;
		av_init_packet(&packet);
		packet.data = NULL;
		packet.size = 0;

		while (size > 0) {
			uint8_t* parsed_data;
			int parsed_size;

			// Parse the input data to packet
			int ret = av_parser_parse2(parser, codecCtx, &packet.data, &packet.size,
				data, size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
			if (ret < 0) {
				break;
			}

			data += ret;
			size -= ret;

			// If we have a packet, decode it
			if (packet.size) {
				if (avcodec_send_packet(codecCtx, &packet) < 0) {
					continue;
				}

				AVFrame* frame = av_frame_alloc();
				while (avcodec_receive_frame(codecCtx, frame) == 0) {
					std::cout << "Got frame" << std::endl;
					// Process the decoded frame (e.g., display or store it)
					// Use the frame for further processing
				}
				av_frame_free(&frame);
			}
		}
		av_packet_unref(&packet);
		//std::cout << count << std::endl;
		//std::cout << line << std::endl;
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