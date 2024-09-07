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
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#include <png.h>

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

// Function to save an AVFrame to a PNG file
int save_frame_to_png(AVFrame* frame, std::string filename)
{
	int ret = 0;

	// Open the PNG file for writing
	FILE* fp = fopen(filename.c_str(), "wb");
	if (!fp)
	{
		return -1;
	}

	// Create the PNG write struct and info struct
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		fclose(fp);
		return -1;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(fp);
		return -1;
	}

	// Set up error handling for libpng
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return -1;
	}

	// Set the PNG file as the output for libpng
	png_init_io(png_ptr, fp);

	// Set the PNG image attributes
	png_set_IHDR(png_ptr, info_ptr, frame->width, frame->height, 8, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Allocate memory for the row pointers and fill them with the AVFrame data
	png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * frame->height);
	for (int y = 0; y < frame->height; y++)
	{
		row_pointers[y] = (png_bytep)(frame->data[0] + y * frame->linesize[0]);
	}

	// Write the PNG file
	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	// Clean up
	free(row_pointers);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);

	return ret;
}

static void save_gray_frame(unsigned char* buf, int wrap, int xsize, int ysize, std::string filename)
{
	FILE* f;
	int i;
	f = fopen(filename.c_str(), "w");
	// writing the minimal required header for a pgm file format
	// portable graymap format -> https://en.wikipedia.org/wiki/Netpbm_format#PGM_example
	fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

	// writing line by line
	for (i = 0; i < ysize; i++)
		fwrite(buf + i * wrap, 1, xsize, f);
	fclose(f);
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

	AVCodecParameters* codec_parameters = avcodec_parameters_alloc();
	codec_parameters->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_parameters->width = 720;
	codec_parameters->height = 1280;
	codec_parameters->profile = FF_PROFILE_H264_BASELINE;

	AVCodecContext* codec_context = avcodec_alloc_context3(codec);
	if (!codec_context) {
		fprintf(stderr, "Could not allocate video codec context\n");
		return;
	}

	if (avcodec_parameters_to_context(codec_context, codec_parameters) < 0)
	{
		fprintf(stderr, "failed to copy codec params to codec context");
		return;
	}

	if (avcodec_open2(codec_context, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		return;
	}

	std::string record_bash_base64 = base64::to_base64(std::string(reinterpret_cast<char*>(screen_record_bash), sizeof(screen_record_bash)));
	std::vector<std::string> commands = { "shell", "echo", record_bash_base64 , "|", "base64", "-d", "|", "sh" };

	std::string command_str = fmt::format("adb -s {} {}", device_serial, fmt::join(commands, " "));
	std::cout << command_str << std::endl;
	boost::process::ipstream pipe_stream;
	boost::process::child c(command_str, boost::process::std_out > pipe_stream);

	int frame_number = 0;

	std::string line;
	while (pipe_stream && std::getline(pipe_stream, line)/* && !line.empty()*/) {
		line = ReplaceAll(line, "\r\n", "\n");

		const uint8_t* data = reinterpret_cast<const uint8_t*>(line.data());
		int size = static_cast<int>(line.size());

		AVPacket *input_packet = av_packet_alloc();
		if (!input_packet)
		{
			return;
		}

		while (size > 0) {
			// Parse the input data to packet
			int ret = av_parser_parse2(parser, codec_context, &input_packet->data, &input_packet->size,
				data, size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
			if (ret < 0) {
				break;
			}

			data += ret;
			size -= ret;

			if (!input_packet->size) {
				continue;
			}

			// Send the packet to the decoder
			if (avcodec_send_packet(codec_context, input_packet) < 0) {
				continue;
			}

			AVFrame* input_frame = av_frame_alloc();
			while (avcodec_receive_frame(codec_context, input_frame) >= 0) {
				// Check if the frame is a planar YUV 4:2:0, 12bpp
				// That is the format of the provided .mp4 file
				// RGB formats will definitely not give a gray image
				// Other YUV image may do so, but untested, so give a warning
				if (input_frame->format != AV_PIX_FMT_YUV420P)
				{
					std::cout << "Warning: the generated file may not be a grayscale image, but could e.g. be just the R component if the video format is RGB";
				}

				if (false) {
					// To create the PNG files, the AVFrame data must be translated from YUV420P format into RGB24
					struct SwsContext* sws_ctx = sws_getContext(
						input_frame->width, input_frame->height, static_cast<AVPixelFormat>(input_frame->format),
						input_frame->width, input_frame->height, AV_PIX_FMT_RGB24,
						SWS_BILINEAR, NULL, NULL, NULL);

					AVFrame* rgb_frame = av_frame_alloc();

					// Set the properties of the output AVFrame
					rgb_frame->format = AV_PIX_FMT_RGB24;
					rgb_frame->width = input_frame->width;
					rgb_frame->height = input_frame->height;

					int ret = av_frame_get_buffer(rgb_frame, 0);
					if (ret < 0)
					{
						return;
					}
					ret = sws_scale(sws_ctx, input_frame->data, input_frame->linesize, 0, input_frame->height, rgb_frame->data, rgb_frame->linesize);
					if (ret < 0)
					{
						return;
					}
					save_frame_to_png(rgb_frame, fmt::format("frame-{}.png", frame_number++));

					av_frame_free(&rgb_frame);
					sws_freeContext(sws_ctx);
				}
				else {
					save_gray_frame(input_frame->data[0], input_frame->linesize[0], input_frame->width, input_frame->height, fmt::format("frame-{}.pgm", frame_number++));
				}
			}
			av_frame_free(&input_frame);
		}
		av_packet_unref(input_packet);
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
	get_size(device_serial);
	record(device_serial);

	std::cout << "Done";
	return 0;
}