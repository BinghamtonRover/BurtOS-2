#include <modules/video_feed_viewer.hpp>

#include <nanogui/nanogui.h>
#include <iostream>

VideoFeedViewer* vid_feed = nullptr;

VideoFeedViewer::VideoFeedViewer(nanogui::Screen *screen) :
	nanogui::Window(screen, "Rover Feed"),
	decoder()
{
	vid_feed = this;

	set_position(nanogui::Vector2i(15,15));
	set_layout(new nanogui::GroupLayout());
	set_size(nanogui::Vector2i(866, 528));

	video_widget = new nanogui::ImageView(this);
	video_widget->set_size(nanogui::Vector2i(856,480));

	perform_layout(screen->nvg_context());
}

void VideoFeedViewer::update_frame_STATIC(int stream, net::Frame& frame){
	vid_feed->update_frame(stream, frame);
}

void VideoFeedViewer::update_frame(int stream, net::Frame& frame) {
	uint8_t next_frame_buffer[Decoder::CAMERA_HEIGHT*Decoder::CAMERA_WIDTH*3];

	try {
		decoder.decode_frame(frame, next_frame_buffer);
	}
	catch(std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return;
	}

	nanogui::Texture* next_frame = new nanogui::Texture(
		nanogui::Texture::PixelFormat::RGB,
		nanogui::Texture::ComponentFormat::UInt8,
		nanogui::Vector2i(Decoder::CAMERA_WIDTH,Decoder::CAMERA_HEIGHT),
		nanogui::Texture::InterpolationMode::Bilinear,
		nanogui::Texture::InterpolationMode::Nearest,
		nanogui::Texture::WrapMode::ClampToEdge,
		(uint8_t) 1,
		nanogui::Texture::TextureFlags::ShaderRead,
		false
	);

	next_frame->upload((uint8_t *) next_frame_buffer);
	video_widget->set_image(next_frame);
	video_widget->center();
	video_widget->set_scale((float)856/Decoder::CAMERA_WIDTH);
}

