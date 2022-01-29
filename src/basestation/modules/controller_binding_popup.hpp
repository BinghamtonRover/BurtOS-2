#pragma once

#include <nanogui/nanogui.h>
#include "../controls/controller_manager.hpp"

class BindingPopup : public nanogui::Window {
	private:
		int joystick_id;
		int axis_idx;
	public:
		BindingPopup(nanogui::Screen* screen, ControllerManager& ctrl_manager, int joystick_id = -1, int axis_idx = 0);
		inline void set_selection(int joystick_id, int axis_idx) {
			joystick_id = joystick_id;
			axis_idx = axis_idx;
		}
		inline void clear_selection() {
			joystick_id = -1;
		}
		inline int joystick_id() const {
			return joystick_id;
		}
		inline int axis_idx() const {
			return axis_idx;
		}
};