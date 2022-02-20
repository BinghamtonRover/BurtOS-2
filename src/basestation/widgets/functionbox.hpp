#pragma once

#include <nanogui/textbox.h>
#include <functional>

/*
	A type of text entry box that raises events upon certain user actions:
	SUBMIT - Pressed enter
	SCROLL_UP/SCROLL_DOWN - Arrow keys
	CANCEL - Ctrl-C pressed (unless text is selected, then it copies the text)
*/
class FunctionBox : public nanogui::TextBox {
public:
	enum Action {
			SUBMIT,
			SCROLL_UP,
			SCROLL_DOWN,
			CANCEL
	};
	
protected:
	std::function<void(Action)> _action;
	std::function<bool(const std::string&)> validate_callback;
public:
	// ActionTextBox needs no special ctor; expose all TextBox defaults:
	using nanogui::TextBox::TextBox;

	bool keyboard_event(int key, int scancode, int key_act, int modifiers);
	bool keyboard_character_event(unsigned int codepoint);

	inline void set_action_callback(const std::function<void(Action)>& act) {
		_action = act;
	}
	inline void action(Action a) {
		if (_action) _action(a);
	}
	// Called to check user input. Should return true if the input is good
	inline void set_verify_callback(const std::function<bool(const std::string&)>& v) {
		validate_callback = v;
	}
	
};
