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
		std::vector<AxisAction> _actions;
		std::function<void(Controller&)> device_connected_callback;
		int hw_cfg = 0;

		static void glfw_joystick_callback(int joystick_id, int event);
		static ControllerManager* main_controller_manager;
	public:
		ControllerManager();
		
		// Set GLFW callbacks and scan devices. Must initialize GLFW first
		void init();

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

		// Run an event when the controller is connected.
		// If the callback is set before calling init(), this will also run for each initial device
		inline void on_device_connect(const std::function<void(Controller&)>& callback) { device_connected_callback = callback; }

		const AxisAction& find_action(const std::string& name) const;

		inline decltype(_devices)& devices() { return _devices; }
		inline const decltype(_actions)& actions() { return _actions; }
		// The hardware config index increments on any controller event (connect, disconnect)
		inline int hw_config_idx() const { return hw_cfg; }

};
