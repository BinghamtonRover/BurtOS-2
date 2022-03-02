#include <modules/video_feed_viewer.hpp>

#include <nanogui/nanogui.h>
#include <iostream>
#include <widgets/layouts/simple_column.hpp>
#include <widgets/layouts/simple_row.hpp>

VideoFeedViewer* vid_feed = nullptr;

nanogui::Vector2i VideoFeedViewer::preferred_size(NVGcontext* ctx) const {
	return nanogui::Vector2i(
		m_fixed_size.x() ? m_fixed_size.x() : m_size.x(),
		m_fixed_size.y() ? m_fixed_size.y() : m_size.y()
	);
}

VideoFeedViewer::VideoFeedViewer(nanogui::Widget* parent) :
	gui::Window(parent, "Rover Feed", true),
	decoder()
{
	vid_feed = this;

	set_layout(new gui::SimpleColumnLayout(6, 6, 6, gui::SimpleColumnLayout::HorizontalAnchor::STRETCH));

	video_widget = new nanogui::ImageView(this);
	
	set_size(nanogui::min(nanogui::Vector2i(1280, 720), parent->size() - 100));
	parent->perform_layout(screen()->nvg_context());
	center();
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
}

