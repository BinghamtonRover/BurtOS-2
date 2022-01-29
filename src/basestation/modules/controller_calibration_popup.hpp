#pragma once

#include <nanogui/nanogui.h>
#include "../controls/controller_manager.hpp"

// Window with fine details for an axis
// This window will never close itself, but may go invisible.
// The creator is responsible for closing it when no longer needed
class CalibrationPopup : public nanogui::Window {
	private:
		// Each configurable value will use a slider and textbox
		struct ConfigValue {
			nanogui::Slider* slider;
			nanogui::TextBox* text;
		};
		nanogui::ProgressBar* cal_value_bar;
		nanogui::TextBox* cal_value_box;
		nanogui::ProgressBar* raw_value_bar;
		nanogui::TextBox* raw_value_box;
		ConfigValue center;
		ConfigValue deadzone;
		ConfigValue minimum;
		ConfigValue maximum;
		ControllerManager& ctrl_manager;
		int selected_joystick_id;
		int selected_axis_idx;

		// Shorthand to return the selected axis and raise an exception if invalid
		// Note that nanogui event handlers catch/ignore exceptions by default
		JoystickAxis& selected_axis();
	public:
		CalibrationPopup(nanogui::Screen* parent, ControllerManager& ctrl_manager, int joystick_id = -1, int axis_idx = 0);
		virtual void draw(NVGcontext* ctx);
		inline void set_selection(int joystick_id, int axis_idx) {
			selected_joystick_id = joystick_id;
			selected_axis_idx = axis_idx;
		}
		inline void clear_selection() {
			selected_joystick_id = -1;
		}
		inline int joystick_id() const {
			return selected_joystick_id;
		}
		inline int axis_idx() const {
			return selected_axis_idx;
		}
};