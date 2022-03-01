#pragma once

#include <turbojpeg.h>
#include <stream.hpp>

class Decoder {
	private:
		tjhandle jpeg_decompressor;
	public:
		Decoder();
		~Decoder();
		void decode_frame(net::Frame& frame, unsigned char * out_buffer);
		
		// TODO: Retrieve individually for each stream
		constexpr static unsigned int CAMERA_WIDTH = 1280;
		constexpr static unsigned int CAMERA_HEIGHT = 720;
};