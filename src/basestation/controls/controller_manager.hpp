#pragma once

#include <vector>
#include <string>
#include <functional>
#include <array>
#include <nanogui/opengl.h>

#include "controller.hpp"

class ControllerManager {
	private:
		constexpr static std::size_t MAX_DEVICES = GLFW_JOYSTICK_LAST + 1;
		std::array<Controller, MAX_DEVICES> _devices;
		std::vector<AxisAction> actions;

		static void glfw_joystick_callback(int joystick_id, int event);
	public:
		ControllerManager();
		~ControllerManager();

		// Read axis values from GLFW and call action callbacks. Call frequently (main loop)
		void update_controls();

		// Look for hardware changes and update device configuration.
		// Called automatically upon initialization or when device config changes
		void scan_devices();

		// Register a type of action that can be bound to a controller axis
		// @param name Identifies this action. Must be unique
		// @param action_callback Function to call when the axis moves.
		//		Value is always between -1.0 and 1.0 (excepting final_value)
		// @param final_value Transmitted to action_callback if the axis device disconnects.
		void add_axis_action(const std::string& name, const std::function<void(float)>& action_callback, float final_value);

		inline decltype(_devices)& devices() { return _devices; }

};
