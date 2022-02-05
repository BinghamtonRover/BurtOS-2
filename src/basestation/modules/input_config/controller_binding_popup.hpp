#pragma once

#include <nanogui/nanogui.h>
#include <controls/controller_manager.hpp>

class BindingPopup : public nanogui::Window {
	private:
		int sel_joystick_id;
		int sel_axis_idx;
		ControllerManager& ctrl_manager;
		nanogui::ComboBox* bind_selector;
	public:
		BindingPopup(nanogui::Screen* screen, ControllerManager& ctrl_manager, int joystick_id = -1, int axis_idx = 0);
		void set_selection(int new_joystick_id, int new_axis_idx);

		virtual void draw(NVGcontext* ctx);

		inline void clear_selection() {
			sel_joystick_id = -1;
		}
		inline int joystick_id() const {
			return sel_joystick_id;
		}
		inline int axis_idx() const {
			return sel_axis_idx;
		}
};