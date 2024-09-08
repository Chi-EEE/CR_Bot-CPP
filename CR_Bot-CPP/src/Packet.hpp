#pragma once

#include <libavcodec/packet.h>

class Packet
{
public:
	Packet(int size) {
		this->ptr = av_packet_alloc();
		av_new_packet(this->ptr, size);
		this->ptr->dts = AV_NOPTS_VALUE;
	}

	~Packet() {
		av_packet_unref(this->ptr);
	}

	AVPacket* ptr = nullptr;
};