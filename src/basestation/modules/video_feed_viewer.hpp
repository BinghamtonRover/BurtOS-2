#pragma once

#include <modules/video_decoder/decoder.hpp>
#include <nanogui/window.h>

class VideoFeedViewer : public nanogui::Window {
	public:
		VideoFeedViewer(nanogui::Screen* screen);
		static void update_frame_STATIC(int stream, net::Frame& frame);
		void update_frame(int stream, net::Frame& frame);
	protected:
		nanogui::ImageView* video_widget;
	private:
		Decoder decoder;
};
