#include <basestation_screen.hpp>
#include <basestation.hpp>

#include <modules/console.hpp>
#include <modules/video_feed_viewer.hpp>

ScreenPositioning::ScreenPositioning(const nanogui::Vector2i& size, const nanogui::Vector2i& window_pos, int monitor, bool use_fullscreen) :
	size(size),
	window_pos(window_pos),
	monitor(monitor),
	use_fullscreen(use_fullscreen) {

}

BasestationScreen::BasestationScreen(const ScreenPositioning& pos)
	: nanogui::Screen(pos.size, "Base Station - Binghamton University Rover Team", true, pos.use_fullscreen),
	position(pos) {

	perform_layout();
	draw_all();
	set_visible(true);
}

void BasestationScreen::set_fullscreen(GLFWmonitor* monitor) {
	if (!monitor) {
		monitor = glfwGetPrimaryMonitor();
	}
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	glfwGetWindowPos(m_glfw_window, &position.window_pos.x(), &position.window_pos.y());

	glfwSetWindowMonitor(m_glfw_window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
	m_fullscreen = true;
	position.use_fullscreen = true;

	resize_callback_event(mode->width, mode->height);
}

void BasestationScreen::set_fullscreen(int monitor) {
	
	m_fullscreen = true;
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);

	if (monitor < 0 || monitor >= count) {
		// If the provided monitor is invalid, attempt to use the preferred monitor
		// specified in the position config
		GLFWmonitor* use_monitor = nullptr;
		if (position.monitor >= 0 && position.monitor < count) {
			use_monitor = monitors[position.monitor];
		}
		set_fullscreen(use_monitor);
	} else {
		position.monitor = monitor;
		set_fullscreen(monitors[monitor]);
	}
}

void BasestationScreen::set_windowed(int w, int h) {
	if (w < 0 || h < 0) {
		w = position.size.x();
		h = position.size.y();
	}
	glfwSetWindowMonitor(m_glfw_window, nullptr, position.window_pos.x(), position.window_pos.y(), w, h, GLFW_DONT_CARE);
	m_fullscreen = false;
	resize_callback_event(w, h);
}

bool BasestationScreen::keyboard_event(int key, int scancode, int action, int mods) {
	if (Screen::keyboard_event(key, scancode, action, mods)) {
		return true;
	}
	bool handled = false;

	/*
		Future: Key events will use an event manager for both efficient handling and for
		runtime flexibility.

		For now, these hard-coded bindings get us started

		Open Console: ~
		Toggle Fullscreen: F11
		New Window: Ctrl + N
	
	*/

	if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
		new Console(this);
		handled = true;
	} else if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
		if (m_fullscreen) {
			set_windowed(position.size);
		} else {
			set_fullscreen(preferred_monitor);
		}
		handled = true;
	} else if (key == GLFW_KEY_N && action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL)) {
		Basestation::get().add_screen(new BasestationScreen());
		handled = true;
	} else if (key == GLFW_KEY_C) {
        auto feed_view = new VideoFeedViewer(this);
        Basestation::get().set_video_callback(VideoFeedViewer::update_frame_STATIC);

        handled = true;
    }

	return handled;
}

bool BasestationScreen::resize_event(const nanogui::Vector2i& size) {
	bool ret = nanogui::Screen::resize_event(size);

	// If the resize left any windows offscreen, move them back
	for (auto wnd : m_children) {
		wnd->set_position(nanogui::max(wnd->position(), nanogui::Vector2i(0)));
		wnd->set_position(nanogui::min(wnd->position(), m_size - wnd->size()));
	}

	if (!m_fullscreen) {
		position.size = m_size;
	}

	return ret;
}

void BasestationScreen::move_here(nanogui::Window* w) {
	// Declare a reference so dispose_window() doesn't delete w
	nanogui::ref w_ref(w);

	nanogui::Screen* old_screen = w->screen();
	if (old_screen == this)
		return;
	
	for (nanogui::Widget* child : w->children()) {
		if (child->focused()) {
			old_screen->update_focus(nullptr);
			break;
		}
	}
	old_screen->dispose_window(w);
	add_child(w);

	// Keep the same position unless it is out of the new screen's bounds
	w->set_position(nanogui::max(w->position(), nanogui::Vector2i(0)));
	w->set_position(nanogui::min(w->position(), m_size - w->size()));


}
