#pragma once

#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
extern "C" {
#include <libavutil/frame.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
}

#include "VideoFormat.hpp"
#include "VideoPlane.h"

class VideoFrame
{
public:
	VideoFrame(int width = 0, int height = 0, std::string format = "yuv420p")
	{
		this->ptr = av_frame_alloc();
		this->_init(av_get_pix_fmt(format.c_str()), width, height);
	}
	~VideoFrame() {
		if (this->ptr != nullptr)
			av_frame_free(&this->ptr);
		if (this->_buffer != nullptr)
			av_freep(&this->_buffer);
	}
	void _init(AVPixelFormat format, int width, int height) {
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
		this->format = VideoFormat(av_get_pix_fmt_name(static_cast<AVPixelFormat>(this->ptr->format)), this->ptr->width, this->ptr->height);
	}

	void _copy_internal_attributes(VideoFrame* frame) {
		this->ptr->format = frame->ptr->format;
		this->ptr->width = frame->ptr->width;
		this->ptr->height = frame->ptr->height;
		this->ptr->ch_layout = frame->ptr->ch_layout;
	}

	std::vector<VideoPlane> planes() {
		int max_plane_count = 0;
		for (int i = 0; i < this->format.ptr->nb_components; i++) {
			int count = this->format.components[i].ptr->plane + 1;
			if (max_plane_count < count) {
				max_plane_count = count;
			}
		}
		if (this->format.ptr->name == "pal8") {
			max_plane_count = 2;
		}
		std::vector<VideoPlane> planes;
		int plane_count = 0;
		while (plane_count < max_plane_count && this->ptr->extended_data[plane_count]) {
			VideoPlane plane(this, plane_count);
			planes.push_back(plane);
			plane_count++;
		}
		return planes;
	}

	void to_image() {
		std::vector<VideoPlane> planes = this->planes();
		VideoPlane& plane = planes[0];

		const uint8_t* i_buf = this->ptr->data[plane.index];
		size_t i_pos = 0;
		size_t i_stride = this->ptr->linesize[plane.index];

		size_t o_pos = 0;
		size_t o_stride = plane.width * 3;
		size_t o_size = plane.height * o_stride;
		std::vector<uint8_t> o_buf(o_size);

		while (o_pos < o_size) {
			std::memcpy(&o_buf[o_pos], &i_buf[i_pos], o_stride);
			i_pos += i_stride;
			o_pos += o_stride;
		}

		cv::Mat img(plane.height, plane.width, CV_8UC3, o_buf.data());
		cv::imwrite("frame.png", img);
	}

	AVFrame* ptr = nullptr;
	VideoFormat format;
private:
	uint8_t* _buffer = nullptr;
};