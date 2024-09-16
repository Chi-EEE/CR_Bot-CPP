#pragma once

#include <optional>
#include <memory>

extern "C" {
#include <libswscale/swscale.h>
}

#include "VideoFrame.hpp"

namespace av {
	enum class AVInterpolation {
		FAST_BILINEAR = SWS_FAST_BILINEAR,
		BILINEAR = SWS_BILINEAR,
		BICUBIC = SWS_BICUBIC,
		X = SWS_X,
		POINT = SWS_POINT,
		AREA = SWS_AREA,
		BICUBLIN = SWS_BICUBLIN,
		GAUSS = SWS_GAUSS,
		SINC = SWS_SINC,
		LANCZOS = SWS_LANCZOS,
		SPLINE = SWS_SPLINE
	};

	class VideoReformatter
	{
	public:
		VideoReformatter() {
		}
		~VideoReformatter() {
			if (this->ptr != nullptr)
				sws_freeContext(this->ptr);
		}

		std::unique_ptr<VideoFrame> reformat(
			VideoFrame* frame,
			int width = 0,
			int height = 0,
			AVPixelFormat format = AV_PIX_FMT_NONE,
			std::optional<AVColorSpace> src_colorspace = std::nullopt,
			std::optional<AVColorSpace> dst_colorspace = std::nullopt,
			std::optional<AVInterpolation> interpolation = std::nullopt,
			int src_color_range = 0,
			int dst_color_range = 0
		) {
			if (format == AV_PIX_FMT_NONE) {
				return nullptr;
			}

			AVColorSpace c_src_colorspace = src_colorspace.value_or(frame->ptr->colorspace);
			AVColorSpace c_dst_colorspace = dst_colorspace.value_or(frame->ptr->colorspace);
			AVInterpolation c_interpolation = interpolation.value_or(AVInterpolation::BILINEAR);
			int c_src_color_range = src_color_range;
			int c_dst_color_range = dst_color_range;
			return std::move(this->_reformat(frame,
				width,
				height,
				format,
				c_src_colorspace,
				c_dst_colorspace,
				c_interpolation,
				c_src_color_range,
				c_dst_color_range
			));
		}
	private:
		std::unique_ptr<VideoFrame> _reformat(
			VideoFrame* frame,
			int width,
			int height,
			AVPixelFormat dst_format,
			AVColorSpace src_colorspace,
			AVColorSpace dst_colorspace,
			AVInterpolation interpolation,
			int src_color_range,
			int dst_color_range
		) {
			if (
				dst_format == frame->ptr->format &&
				width == frame->ptr->width &&
				height == frame->ptr->height &&
				dst_colorspace == src_colorspace &&
				src_color_range == dst_color_range
				)
			{
				std::unique_ptr<VideoFrame> new_frame = std::make_unique<VideoFrame>();
				new_frame->_copy_internal_attributes(frame);
				new_frame->_init(dst_format, width, height);
				return std::move(new_frame);
			}

			this->ptr = sws_getCachedContext(
				this->ptr,
				frame->ptr->width,
				frame->ptr->height,
				static_cast<AVPixelFormat>(frame->ptr->format),
				width,
				height,
				dst_format,
				static_cast<int>(interpolation),
				NULL,
				NULL,
				NULL);

			// We want to change the colorspace / color_range transforms.
			// We do that by grabbing all of the current settings, changing a
			// couple, and setting them all. We need a lot of state here.
			int* inv_tbl = nullptr;
			int* tbl = nullptr;
			int src_colorspace_range = 0, dst_colorspace_range = 0;
			int brightness = 0, contrast = 0, saturation = 0;
			int ret;

			if (src_colorspace != dst_colorspace || src_color_range != dst_color_range) {
				ret = sws_getColorspaceDetails(
					this->ptr,
					&inv_tbl,
					&src_colorspace_range,
					&tbl,
					&dst_colorspace_range,
					&brightness,
					&contrast,
					&saturation
				);

				// Grab the coefficients for the requested transforms.
					// The inv_table brings us to linear, and `tbl` to the new space.
				if (src_colorspace != SWS_CS_DEFAULT)
					inv_tbl = const_cast<int*>(sws_getCoefficients(src_colorspace));
				if (dst_colorspace != SWS_CS_DEFAULT)
					tbl = const_cast<int*>(sws_getCoefficients(dst_colorspace));

				// Apply!
				ret = sws_setColorspaceDetails(
					this->ptr,
					inv_tbl,
					src_color_range,
					tbl,
					dst_color_range,
					brightness,
					contrast,
					saturation
				);
			}

			// Create a new frame to hold the reformatted data.
			std::unique_ptr<VideoFrame> new_frame = std::make_unique<VideoFrame>();
			new_frame->_copy_internal_attributes(frame);
			new_frame->_init(dst_format, width, height);

			// Perform the scaling operation.
			sws_scale(
				this->ptr,
				frame->ptr->data,
				frame->ptr->linesize,
				0, // slice Y
				frame->ptr->height,
				new_frame->ptr->data,
				new_frame->ptr->linesize
			);

			return std::move(new_frame);
		}
		SwsContext* ptr = nullptr;
	};
}
