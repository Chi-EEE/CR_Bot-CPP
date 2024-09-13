#include "VideoFormatComponent.h"
#include "VideoFormat.hpp"

namespace av {
	VideoFormatComponent::VideoFormatComponent(VideoFormat* format, size_t index) {
		this->format = format;
		this->index = index;
		this->ptr = &format->ptr->comp[index];
	}
}
