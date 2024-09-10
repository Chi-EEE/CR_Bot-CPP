#include "VideoFormatComponent.h"
#include "VideoFormat.hpp"

VideoFormatComponent::VideoFormatComponent(VideoFormat* format, size_t index) {
	this->format = format;
	this->index = index;
	this->ptr = &format->ptr->comp[index];
}