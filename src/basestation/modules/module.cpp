#include <modules/module.hpp>

#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/icons.h>
#include <nanogui/button.h>

#include <basestation.hpp>

gui::Module::Module(nanogui::Widget* parent, const std::string& title, bool resizable, bool minimizable, bool closable)
	: nanogui::Window(parent, title),
	m_resize_dir{ResizeSide::NONE},
	m_min_size(nanogui::Vector2i(m_theme->m_window_header_height)),
	m_resizable(resizable),
	m_resize(false),
	m_closable(closable),
	m_minimizable(minimizable) {

	if (m_minimizable) {
		button_panel()->add<nanogui::Button>("", FA_WINDOW_MINIMIZE)->set_callback([this] {
			set_visible(false);
		});
	}

	if (m_closable) {
		button_panel()->add<nanogui::Button>("", FA_WINDOW_CLOSE)->set_callback([this] {
			close();
		});
	}

}

void gui::Module::close() {
	Basestation::async([this](Basestation&) {
		if (nanogui::Screen* screen = dynamic_cast<nanogui::Screen*>(m_parent)) {
			screen->dispose_window(this);
		} else {
			m_parent->remove_child(this);
		}
	});
}

bool gui::Module::mouse_drag_event(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int mods) {
	if (m_resize && (button & (1 << GLFW_MOUSE_BUTTON_1))) {

		auto screen = this->screen();

		auto abs_pos = screen->absolute_position();
		if (p.x() < abs_pos.x() || p.x() > abs_pos.x() + screen->size().x() ||
				p.y() < abs_pos.y() || p.y() > abs_pos.y() + screen->size().y()) {

			m_resize = false;
			return false;
		}

		const nanogui::Vector2i lower_right_corner(m_pos + m_size);
		const nanogui::Vector2i upper_left_corner(m_pos);
		NVGcontext* ctx = screen->nvg_context();
		bool resized = false;

		if (m_resize_dir[0] == ResizeSide::RIGHT) {
			if ((rel.x() > 0 && p.x() >= lower_right_corner.x()) || (rel.x() < 0)) {
				m_size.x() += rel.x();
				resized = true;
			}
		} else if (m_resize_dir[0] == ResizeSide::LEFT) {
			if ((rel.x() < 0 && p.x() <= upper_left_corner.x()) || (rel.x() > 0)) {
				m_size.x() += -rel.x();
				m_size = max(m_size, m_min_size);
				m_pos = lower_right_corner - m_size;
				resized = true;
			}
		}

		if (m_resize_dir[1] == ResizeSide::DOWN) {
			if ((rel.y() > 0 && p.y() >= lower_right_corner.y()) || (rel.y() < 0)) {
				m_size.y() += rel.y();
				resized = true;
			}
		}
		m_size = max(m_size, m_min_size);
		

		if (resized) perform_layout(ctx);

		return true;
	}

	return nanogui::Window::mouse_drag_event(p, rel, button, mods);
}

bool gui::Module::mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) {
	if (m_resizable) {
		bool hresize = check_horizontal_resize(p) != ResizeSide::NONE;
		bool vresize = check_vertical_resize(p) != ResizeSide::NONE;
		if (hresize && vresize) {
			m_cursor = nanogui::Cursor::Crosshair;
		} else if (hresize) {
			m_cursor = nanogui::Cursor::HResize;
		} else if (vresize) {
			m_cursor = nanogui::Cursor::VResize;
		} else {
			m_cursor = nanogui::Cursor::Arrow;
		}
		return true;
	}
   
	return nanogui::Window::mouse_motion_event(p, rel, button, modifiers);
}

gui::Module::ResizeSide gui::Module::check_vertical_resize(const nanogui::Vector2i &mouse_pos) {
	nanogui::Vector2i lower_right_corner = absolute_position() + size();

	// Do not check for resize area on top of the window. It is to prevent conflict drag and resize event.
	if (mouse_pos.y() >= lower_right_corner.y() - RESIZE_ZONE_RADIUS && mouse_pos.y() <= lower_right_corner.y()) {
		return ResizeSide::DOWN;
	}

	return ResizeSide::NONE;
}

gui::Module::ResizeSide gui::Module::check_horizontal_resize(const nanogui::Vector2i &mouse_pos) {
	auto abs_pos = absolute_position();
	nanogui::Vector2i lower_right_corner = abs_pos + size();
	int header_lower_left_corner_y = abs_pos.y() + m_theme->m_window_header_height;

	if (mouse_pos.y() > header_lower_left_corner_y) {

		if (mouse_pos.x() <= abs_pos.x() + RESIZE_ZONE_RADIUS && mouse_pos.x() >= abs_pos.x()) return ResizeSide::LEFT;
		else if (mouse_pos.x() >= lower_right_corner.x() - RESIZE_ZONE_RADIUS && mouse_pos.x() <= lower_right_corner.x()) return ResizeSide::RIGHT;
	}


	return ResizeSide::NONE;
}

bool gui::Module::mouse_button_event(const nanogui::Vector2i& pos, int button, bool down, int mods) {
	if (button == GLFW_MOUSE_BUTTON_1) {
		m_resize = false;
		if (m_resizable && !m_drag && down) {
			m_resize_dir[0] = check_horizontal_resize(pos);
			m_resize_dir[1] = check_vertical_resize(pos);
			m_resize = m_resize_dir[0] != ResizeSide::NONE || m_resize_dir[1] != ResizeSide::NONE;
			if (m_resize)
				return true;
		}
	}

	if (nanogui::Window::mouse_button_event(pos, button, down, mods))
		return true;

	return false;
}
