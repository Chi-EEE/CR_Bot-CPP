
// Function to save an AVFrame to a PNG file
int save_frame_to_png(AVFrame* frame, std::string filename)
{
	struct SwsContext* sws_ctx = sws_getContext(
		frame->width, frame->height, static_cast<AVPixelFormat>(frame->format),
		frame->width, frame->height, AV_PIX_FMT_RGB24,
		SWS_BILINEAR, NULL, NULL, NULL);

	AVFrame* rgb_frame = av_frame_alloc();

	// Set the properties of the output AVFrame
	rgb_frame->format = AV_PIX_FMT_RGB24;
	rgb_frame->width = frame->width;
	rgb_frame->height = frame->height;

	int ret = av_frame_get_buffer(rgb_frame, 0);
	if (ret < 0)
	{
		return -1;
	}

	ret = sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, rgb_frame->data, rgb_frame->linesize);
	if (ret < 0)
	{
		return -1;
	}

	// Open the PNG file for writing
	FILE* fp = fopen(filename.c_str(), "wb");
	if (!fp)
	{
		return -1;
	}

	// Create the PNG write struct and info struct
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		fclose(fp);
		return -1;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(fp);
		return -1;
	}

	// Set up error handling for libpng
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return -1;
	}

	// Set the PNG file as the output for libpng
	png_init_io(png_ptr, fp);

	// Set the PNG image attributes
	png_set_IHDR(png_ptr, info_ptr, frame->width, frame->height, 8, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Allocate memory for the row pointers and fill them with the AVFrame data
	png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * frame->height);
	for (int y = 0; y < frame->height; y++)
	{
		row_pointers[y] = (png_bytep)(frame->data[0] + y * frame->linesize[0]);
	}

	// Write the PNG file
	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	// Clean up
	free(row_pointers);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);

	av_frame_free(&rgb_frame);
	sws_freeContext(sws_ctx);

	return ret;
}