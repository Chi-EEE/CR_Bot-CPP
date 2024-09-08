#pragma once
#include <libavutil/frame.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>

#include <string>

class VideoFrame
{
public:
	VideoFrame(int width = 0, int height = 0, std::string format = "yuv420p")
	{
		this->ptr = av_frame_alloc();
		this->_init(width, height, av_get_pix_fmt(format.c_str()));
	}
	~VideoFrame() {
		if (this->ptr != nullptr)
			av_frame_free(&this->ptr);
		if (this->_buffer != nullptr)
			av_freep(&this->_buffer);
	}
	void _init(int width, int height, AVPixelFormat format) {
		int res = 0;
		this->ptr->width = width;
		this->ptr->height = height;
		this->ptr->format = format;
		if (width && height) {
			res = av_image_alloc(
				this->ptr->data,
				this->ptr->linesize,
				width,
				height,
				format,
				16);
			this->_buffer = this->ptr->data[0];
		}
		if (res) {
			// err_check
		}
		this->_init_user_attributes();
	}
	void _init_user_attributes() {
		this->format = av_get_pix_fmt(av_get_pix_fmt_name(static_cast<AVPixelFormat>(this->ptr->format)));
	}

	AVFrame* ptr = nullptr;
private:
	uint8_t* _buffer = nullptr;
	AVPixelFormat format;

};