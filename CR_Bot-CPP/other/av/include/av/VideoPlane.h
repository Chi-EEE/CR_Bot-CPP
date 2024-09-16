#pragma once

namespace av {
	class VideoFrame;

	class VideoPlane {
	public:
		VideoPlane(VideoFrame* frame, int index);

		VideoFrame* frame = nullptr;
		int index = 0;
		int width = 0;
		int height = 0;
		int buffer_size = 0;
	};
}
