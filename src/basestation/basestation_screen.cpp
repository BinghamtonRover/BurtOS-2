#include <basestation_screen.hpp>
#include <basestation.hpp>

#include <modules/console.hpp>

BasestationScreen::BasestationScreen()
	: nanogui::Screen(nanogui::Vector2i(DEFAULT_WIDTH, DEFAULT_HEIGHT), "Base Station - Binghamton University Rover Team", true),
	preferred_size(DEFAULT_WIDTH, DEFAULT_HEIGHT) {

	perform_layout();
	draw_all();
	set_visible(true);
}

void BasestationScreen::set_fullscreen(GLFWmonitor* monitor) {
	if (!monitor) {
		monitor = glfwGetPrimaryMonitor();
	}
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	glfwSetWindowMonitor(m_glfw_window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
	m_fullscreen = true;

	resize_callback_event(mode->width, mode->height);
}

void BasestationScreen::set_fullscreen(int monitor) {
	
	m_fullscreen = true;
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);

	if (monitor < 0 || monitor >= count) {
		set_fullscreen(nullptr);
	} else {
		set_fullscreen(monitors[monitor]);
	}
}

void BasestationScreen::set_windowed(int w, int h) {
	if (w < 0 || h < 0) {
		w = preferred_size.x();
		h = preferred_size.y();
	}
	glfwSetWindowMonitor(m_glfw_window, nullptr, 100, 100, w, h, GLFW_DONT_CARE);
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
			set_windowed(preferred_size);
		} else {
			set_fullscreen(preferred_monitor);
		}
		handled = true;
	} else if (key == GLFW_KEY_N && action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL)) {
		Basestation::get().add_screen(new BasestationScreen());
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
		preferred_size = m_size;
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
