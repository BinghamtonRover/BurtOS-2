#pragma once

#include <nanogui/textbox.h>
#include <nanogui/opengl.h>
#include <functional>

/*
	Normal nanogui::TextBox except it can call an action handler when enter is pressed
*/
class ActionTextBox : public nanogui::TextBox {
private:
	std::function<void()> _action;
public:
	// ActionTextBox needs no special ctor; expose all TextBox defaults:
	using nanogui::TextBox::TextBox;

	inline void action() {
		if (_action) _action();
	}
	bool keyboard_event(int key, int scancode, int key_act, int modifiers) override {
		if (focused() && m_editable) {
			if (key == GLFW_KEY_ENTER && (key_act == GLFW_PRESS || key_act == GLFW_REPEAT)) {
				focus_event(false);
				action();
				focus_event(true);
				return true;
			}
		}
		return nanogui::TextBox::keyboard_event(key, scancode, key_act, modifiers);
	}
	void set_action_callback(const std::function<void()>& act) {
		_action = act;
	}
	
};
