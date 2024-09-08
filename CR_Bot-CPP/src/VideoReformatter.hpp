#pragma once

#include "VideoFrame.hpp"
#include <libswscale/swscale.h>

class VideoReformatter
{
public:
	VideoReformatter();
	~VideoReformatter();
	VideoFrame Reformat(VideoFrame frame, int width = 0, int height = 0) {

	}
private:
	VideoFrame _reformat(VideoFrame frame, int width, int height) {
		this->ptr = sws_getCachedContext(
			this->ptr,
			frame.ptr->width,
			frame.ptr->height,
			static_cast<AVPixelFormat>(frame.ptr->format),
			width, height,
			AV_PIX_FMT_RGB24,
			SWS_BILINEAR,
			NULL,
			NULL,
			NULL);

		// We want to change the colorspace / color_range transforms.
		// We do that by grabbing all of the current settings, changing a
		// couple, and setting them all.We need a lot of state here.
		int* inv_tbl;
		int* tbl;
		int src_colorspace_range, dst_colorspace_range;
		int brightness, contrast, saturation;
		int ret;

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
			inv_tbl = sws_getCoefficients(src_colorspace);
		if (dst_colorspace != SWS_CS_DEFAULT)
			tbl = sws_getCoefficients(dst_colorspace);

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

		VideoFrame new_frame;;
		new_frame._copy_internal_attributes(frame);
		new_frame.init(dst_format, width, height);
	}
	SwsContext* ptr;
}