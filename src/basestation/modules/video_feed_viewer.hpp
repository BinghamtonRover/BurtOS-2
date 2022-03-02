#pragma once

#include <video_decoder/decoder.hpp>
#include <widgets/window.hpp>

class VideoFeedViewer : public gui::Window {
	public:
		VideoFeedViewer(nanogui::Widget* parent);
		static void update_frame_STATIC(int stream, net::Frame& frame);
		void update_frame(int stream, net::Frame& frame);

		virtual nanogui::Vector2i preferred_size(NVGcontext* ctx) const override;
		virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override;

	protected:
		nanogui::ImageView* video_widget;
	private:
		constexpr static int MARGIN = 4;
		Decoder decoder;
		bool borderless = false;

		void fit_image();
};
