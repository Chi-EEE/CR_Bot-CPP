#pragma once

#include <string>
#include <vector>

extern "C" {
#include <libavutil/imgutils.h>
}

#include "VideoFormatComponent.h"

namespace av {
	class VideoFormat
	{
	public:
		VideoFormat(std::string name = "yuv420p", int width = 0, int height = 0)
		{
			AVPixelFormat pix_fmt = av_get_pix_fmt(name.c_str());
			this->_init(pix_fmt, width, height);
		}

		~VideoFormat() {}

		void _init(AVPixelFormat pix_fmt, int width, int height) {
			this->pix_fmt = pix_fmt;
			this->ptr = const_cast<AVPixFmtDescriptor*>(av_pix_fmt_desc_get(pix_fmt));
			this->width = width;
			this->height = height;
			for (uint8_t index = 0; index < this->ptr->nb_components; index++) {
				this->components.push_back(VideoFormatComponent(this, index));
			}
		}

		int width = 0;
		int height = 0;
		AVPixelFormat pix_fmt;
		AVPixFmtDescriptor* ptr = nullptr;
		std::vector<VideoFormatComponent> components;
	};
}
