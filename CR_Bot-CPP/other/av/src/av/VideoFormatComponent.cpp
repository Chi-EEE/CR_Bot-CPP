#include "av/VideoFormatComponent.h"
#include "av/VideoFormat.hpp"

namespace av {
	VideoFormatComponent::VideoFormatComponent(VideoFormat* format, size_t index) {
		this->format = format;
		this->index = index;
		this->ptr = &format->ptr->comp[index];
	}
}
