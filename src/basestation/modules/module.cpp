#include <modules/module.hpp>

#include <nanogui/opengl.h>
#include <nanogui/screen.h>

gui::Module::Module(nanogui::Widget* parent, const std::string& title, bool resizable)
	: nanogui::Window(parent, title), m_resize_dir(nanogui::Vector2i(5)), m_min_size(nanogui::Vector2i(m_theme->m_window_header_height)), m_resizable(resizable) {}

bool gui::Module::mouse_drag_event(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int mods) {
	if (nanogui::Window::mouse_drag_event(p, rel, button, mods))
		return true;

	if (m_resizable && m_resize && (button & (1 << GLFW_MOUSE_BUTTON_1))) {
		const nanogui::Vector2i lower_right_corner(m_pos + m_size);
		const nanogui::Vector2i upper_left_corner(m_pos);
		NVGcontext* ctx = screen()->nvg_context();
		bool resized = false;

		if (m_resize_dir.x() == 1) {
			if ((rel.x() > 0 && p.x() >= lower_right_corner.x()) || (rel.x() < 0)) {
				m_size.x() += rel.x();
				resized = true;
			}
		} else if (m_resize_dir.x() == -1) {
			if ((rel.x() < 0 && p.x() <= upper_left_corner.x()) || (rel.x() > 0)) {
				m_size.x() += -rel.x();
				m_size = max(m_size, m_min_size);
				m_pos = lower_right_corner - m_size;
				resized = true;
			}
		}

		if (m_resize_dir.y() == 1) {
			if ((rel.y() > 0 && p.y() >= lower_right_corner.y()) || (rel.y() < 0)) {
				m_size.y() += rel.y();
				resized = true;
			}
		} 
		m_size = max(m_size, m_min_size);

		if (resized) perform_layout(ctx);

		return true;
	}

	return false;
}

bool gui::Module::mouse_motion_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) {
	if (nanogui::Window::mouse_motion_event(p, rel, button, modifiers))
		return true;

	if (m_resizable) {
		int hresize = check_horizontal_resize(p);
		int vresize = check_vertical_resize(p);
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
   
	return false;
}

int gui::Module::check_vertical_resize(const nanogui::Vector2i &mouse_pos) {
	int offset = 20;
	nanogui::Vector2i lower_right_corner = absolute_position() + size();

	// Do not check for resize area on top of the window. It is to prevent conflict drag and resize event.
	if (mouse_pos.y() >= lower_right_corner.y() - offset && mouse_pos.y() <= lower_right_corner.y()) {
		return 1;
	}

	return 0;
}

int gui::Module::check_horizontal_resize(const nanogui::Vector2i &mouse_pos) {
	int offset = 20;
	nanogui::Vector2i lower_right_corner = absolute_position() + size();
	int header_lower_left_corner_y = absolute_position().y() + m_theme->m_window_header_height;

	if (mouse_pos.y() > header_lower_left_corner_y && mouse_pos.x() <= absolute_position().x() + offset && mouse_pos.x() >= absolute_position().x()) return -1;
	else if ( mouse_pos.y() > header_lower_left_corner_y && mouse_pos.x() >= lower_right_corner.x() - offset && mouse_pos.x() <= lower_right_corner.x()) return 1;

	return 0;
}

bool gui::Module::mouse_button_event(const nanogui::Vector2i& pos, int button, bool down, int mods) {
	if (button == GLFW_MOUSE_BUTTON_1) {
		m_resize = false;
		if (m_resizable && !m_drag && down) {
			m_resize_dir.x() = check_horizontal_resize(pos);
			m_resize_dir.y() = check_vertical_resize(pos);
			m_resize = m_resize_dir.x() != 0 || m_resize_dir.y() != 0;
			if (m_resize)
				return true;
		}
	}

	if (nanogui::Window::mouse_button_event(pos, button, down, mods))
		return true;

	return false;
}
