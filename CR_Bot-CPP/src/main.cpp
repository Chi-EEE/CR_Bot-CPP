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

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#include <opencv2/opencv.hpp>
#include <png.h>

#include "video/VideoFrame.hpp"
#include "video/Packet.hpp"

#include "video/VideoReformatter.hpp"
#include "video/VideoFormat.hpp"

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

// Function to save an AVFrame to a PNG file
/*int save_frame_to_png(AVFrame* frame, std::string filename)
{
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
	return 0;
}*/

static void save_gray_frame(unsigned char* buf, int wrap, int xsize, int ysize, std::string filename)
{
	FILE* f;
	int i;
	f = fopen(filename.c_str(), "wb");
	// writing the minimal required header for a pgm file format
	// portable graymap format -> https://en.wikipedia.org/wiki/Netpbm_format#PGM_example
	fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

	// writing line by line
	for (i = 0; i < ysize; i++)
		fwrite(buf + i * wrap, 1, xsize, f);
	fclose(f);
}

static void process(AVCodecParserContext* parser, AVCodecContext* codec_context, int& frame_number, std::array<char, 1> buffer) {
	std::vector<std::unique_ptr<Packet>> packets;

	const uint8_t* in_data = reinterpret_cast<const uint8_t*>(buffer.data());
	int in_size = static_cast<int>(buffer.size());

	unsigned char* out_data;
	int out_size;
	int consumed = 0;

	while (in_size > 0) {
		// Parse the input data to packet
		consumed = av_parser_parse2(parser, codec_context, &out_data, &out_size,
			in_data, in_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

		if (consumed < 0) {
			break;
		}

		if (out_size) {
			std::unique_ptr<Packet> packet = std::make_unique<Packet>(out_size);
			memcpy(packet->ptr->data, out_data, out_size);
			packets.push_back(std::move(packet));
		}

		if (in_size <= 0) {
			// This was a flush. Only one packet should ever be returned.
			break;
		}

		in_data += consumed;
		in_size -= consumed;

		if (in_size <= 0) {
			break;
		}
	}

	if (packets.empty()) {
		return;
	}

	std::unique_ptr<Packet>& packet = packets.back();

	avcodec_send_packet(codec_context, packet->ptr);

	std::vector<std::unique_ptr<VideoFrame>> frames;
	while (true) {
		std::unique_ptr<VideoFrame> frame = std::make_unique<VideoFrame>();

		int ret = avcodec_receive_frame(codec_context, frame->ptr);
		if (ret == AVERROR_EOF) {
			// Decoding finished, no more frames
			break;
		}
		else if (ret == AVERROR(EAGAIN)) {
			// No more frames for now, wait for more input
			break;
		}
		else if (ret < 0) {
			// Handle other errors
			break;
		}

		frames.push_back(std::move(frame));
	}

	for (std::unique_ptr<VideoFrame>& frame : frames) {
		VideoReformatter reformatter;
		std::unique_ptr<VideoFrame> reformatted_frame = reformatter.reformat(frame.get(), 720, 1280, AV_PIX_FMT_BGR24);
		if (!reformatted_frame) {
			continue;
		}
		
		reformatted_frame->to_image();

		//save_frame_to_png(reformatted_frame->ptr, fmt::format("frame_{}.png", frame_number));
		//save_gray_frame(frame->ptr->data[0], frame->ptr->linesize[0], frame->ptr->width, frame->ptr->height, fmt::format("frame_{}.pgm", frame_number));

		frame_number++;
	}
}

static void record(const std::string device_serial) {
	//av_log_set_level(AV_LOG_QUIET);

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

	AVCodecContext* codec_context = avcodec_alloc_context3(codec);
	if (!codec_context) {
		fprintf(stderr, "Could not allocate video codec context\n");
		return;
	}

	if (avcodec_open2(codec_context, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		return;
	}

	std::string record_bash_base64 = base64::to_base64(screen_record_bash);
	std::vector<std::string> commands = { "exec-out", "screenrecord", "--output-format=h264", "-" };

	std::string command_str = fmt::format("adb -s {} {}", device_serial, fmt::join(commands, " "));
	std::cout << command_str << std::endl;

	boost::asio::io_context ioc;
	boost::process::async_pipe pipe(ioc);
	boost::process::child c(command_str, boost::process::std_out > pipe, ioc);

	int frame_number = 0;

	std::array<char, 1> buffer;
	std::function<void()> do_read;
	do_read = [&]() {
		boost::asio::async_read(pipe, boost::asio::buffer(buffer),
			[&](boost::system::error_code ec, std::size_t length) {
				if (!ec) {
					// Process the received line
					process(parser, codec_context, frame_number, buffer);

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