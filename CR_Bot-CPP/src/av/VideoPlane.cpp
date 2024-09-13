#include "VideoPlane.h"
#include "VideoFrame.hpp"

namespace av {
	VideoPlane::VideoPlane(VideoFrame* frame, int index)
	{
		if (frame->ptr->format == AV_PIX_FMT_PAL8 && index == 1) {
			this->width = 256;
			this->height = 1;
			this->buffer_size = 256 * 4;
			return;
		}
		for (int i = 0; i < frame->format.ptr->nb_components; i++) {
			if (frame->format.ptr->comp[i].plane == index) {
				VideoFormatComponent* component = &frame->format.components[i];
				// Not the same
				this->width = frame->ptr->width;
				this->height = frame->ptr->height;
				break;
			}
		}
		this->frame = frame;
		this->buffer_size = std::abs(frame->ptr->linesize[index]) * frame->ptr->height;
	}
}

