#pragma once

#include <nanogui/textbox.h>
#include <nanogui/opengl.h>
#include <functional>

/*
	Normal nanogui::TextBox except it can call an action handler when enter is pressed
*/
class ActionTextBox : public nanogui::TextBox {
public: enum Action {
		SUBMIT,
		SCROLL_UP,
		SCROLL_DOWN,
		CANCEL,
		NEWLINE
	};
	
protected: std::function<void(Action)> _action;
public:
	// ActionTextBox needs no special ctor; expose all TextBox defaults:
	using nanogui::TextBox::TextBox;

	inline void action(Action a) {
		if (_action) _action(a);
	}
	bool keyboard_event(int key, int scancode, int key_act, int modifiers) override {
		if (focused() && m_editable) {
			if (key == GLFW_KEY_ENTER && (key_act == GLFW_PRESS || key_act == GLFW_REPEAT)) {
				focus_event(false);
				action(Action::SUBMIT);
				focus_event(true);
				return true;
			}
		}
		return nanogui::TextBox::keyboard_event(key, scancode, key_act, modifiers);
	}
	void set_action_callback(const std::function<void(Action)>& act) {
		_action = act;
	}
	
};
