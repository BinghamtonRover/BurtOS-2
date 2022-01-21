#include "functionbox.hpp"

#include <nanogui/opengl.h>

bool FunctionBox::keyboard_event(int key, int scancode, int key_act, int modifiers) {
	if (focused() && m_editable) {
		if (key == GLFW_KEY_ENTER && (key_act == GLFW_PRESS || key_act == GLFW_REPEAT)) {
			focus_event(false);
			action(Action::SUBMIT);
			focus_event(true);
			return true;
		} else if (key == GLFW_KEY_UP && (key_act == GLFW_PRESS || key_act == GLFW_REPEAT)) {
			focus_event(false);
			action(Action::SCROLL_UP);
			focus_event(true);
			return true;
		} else if (key == GLFW_KEY_DOWN && (key_act == GLFW_PRESS || key_act == GLFW_REPEAT)) {
			focus_event(false);
			action(Action::SCROLL_DOWN);
			focus_event(true);
			return true;
		} else if (key == GLFW_KEY_C && (modifiers & SYSTEM_COMMAND_MOD) && (key_act == GLFW_PRESS || key_act == GLFW_REPEAT)) {
			// Ctrl-C: If text is selected, assume user means "clipboard copy". Otherwise, propagate Action::CANCEL
			if (!copy_selection()) {
				focus_event(false);
				action(Action::CANCEL);
				focus_event(true);
			}
			return true;
		}
	}
	return nanogui::TextBox::keyboard_event(key, scancode, key_act, modifiers);
}