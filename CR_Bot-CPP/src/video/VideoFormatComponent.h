#pragma once

#include <string>
#include <vector>

extern "C" {
#include <libavutil/imgutils.h>
}

class VideoFormat;

class VideoFormatComponent {
public:
	VideoFormatComponent(VideoFormat* format, size_t index);
	VideoFormat* format = nullptr;
	size_t index = 0;
	AVComponentDescriptor* ptr = nullptr;
};