#include <modules/video_feed_viewer.hpp>

#include <nanogui/nanogui.h>
#include <nanogui/opengl.h>
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

	set_layout(new gui::SimpleColumnLayout(MARGIN, MARGIN, 6, gui::SimpleColumnLayout::HorizontalAnchor::STRETCH));

	video_widget = new nanogui::ImageView(this);
	video_widget->set_border_color(nanogui::Color(0, 0, 0, 0));
	
	set_size(nanogui::min(nanogui::Vector2i(1280, 720), parent->size() - 100));
	parent->perform_layout(screen()->nvg_context());
	center();
}

bool VideoFeedViewer::keyboard_event(int key, int scancode, int action, int modifiers) {
	if (gui::Window::keyboard_event(key, scancode, action, modifiers)) {
		return true;
	}
	bool handled = false;

	if (action == GLFW_PRESS && !(modifiers & (GLFW_MOD_CONTROL))) {
		handled = true;
		switch (key) {
			case GLFW_KEY_F:
				borderless = !borderless;
				if (gui::SimpleColumnLayout* layout = dynamic_cast<gui::SimpleColumnLayout*>(m_layout.get())) {
					if (borderless) {
						layout->set_horizontal_margin(0);
						layout->set_vertical_margin(0);
					} else {
						layout->set_horizontal_margin(MARGIN);
						layout->set_vertical_margin(MARGIN);
					}
					perform_layout(screen()->nvg_context());
				}
				break;
			case GLFW_KEY_T: {
				// Fit the image in the current size
				fit_image();
				break;
			}
			case GLFW_KEY_Y: {
				// Resize window to fit the feed
				nanogui::Vector2i ideal_size = video_widget->image()->size();
				if (!borderless) {
					ideal_size += MARGIN;
				}
				ideal_size.y() += m_theme->m_window_header_height;

				auto scr = screen();
				set_size(nanogui::min(ideal_size, scr->size() - 100));
				perform_layout(scr->nvg_context());

				fit_image();
				break;
			}
			default:
				handled = false;
				break;
		}
	}

	return handled;
}

void VideoFeedViewer::fit_image() {
	nanogui::Vector2f scale = static_cast<nanogui::Vector2f>(video_widget->size()) / static_cast<nanogui::Vector2f>(video_widget->image()->size());
	video_widget->set_scale(std::min(scale.x(), scale.y()));
	video_widget->center();
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

