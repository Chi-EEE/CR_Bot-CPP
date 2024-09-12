#pragma once

#include <memory>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include "Packet.hpp"
#include "VideoFrame.hpp"
#include "VideoReformatter.hpp"

class CodecContext {
public:
	CodecContext() {
		const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
		if (!codec) {
			throw std::runtime_error("Codec not found");
		}

		AVCodecParserContext* parser = av_parser_init(codec->id);
		if (!parser) {
			throw std::runtime_error("Parser not found");
		}

		AVCodecContext* codec_context = avcodec_alloc_context3(codec);
		if (!codec_context) {
			throw std::runtime_error("Could not allocate codec context");
		}

		if (avcodec_open2(codec_context, codec, NULL) < 0) {
			throw std::runtime_error("Could not open codec");
		}

		this->parser = parser;
		this->codec_context = codec_context;
	}

	~CodecContext() {
		av_parser_close(this->parser);
		avcodec_free_context(&this->codec_context);
	}

	[[nodiscard]]
	std::vector<std::unique_ptr<Packet>> parse(const uint8_t* buffer, const size_t size) {
		std::vector<std::unique_ptr<Packet>> packets;

		const uint8_t* in_data = buffer;
		size_t in_size = size;

		unsigned char* out_data;
		int out_size;
		int consumed = 0;

		while (in_size > 0) {
			// Parse the input data to packet
			consumed = av_parser_parse2(this->parser, this->codec_context, &out_data, &out_size,
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

		return packets;
	}

	[[nodiscard]]
	std::vector<std::unique_ptr<VideoFrame>> decode(std::unique_ptr<Packet>& packet) {
		avcodec_send_packet(this->codec_context, packet->ptr);

		std::vector<std::unique_ptr<VideoFrame>> frames;
		while (true) {
			std::unique_ptr<VideoFrame> frame = std::make_unique<VideoFrame>();

			int ret = avcodec_receive_frame(this->codec_context, frame->ptr);
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
		return frames;
	}

private:
	AVCodecParserContext* parser;
	AVCodecContext* codec_context;
};