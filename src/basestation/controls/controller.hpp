#pragma once

#include <vector>
#include <string>
#include <functional>
#include <nanogui/opengl.h>

class AxisAction {
	private:
		std::string display_name;
		std::function<void(float)> callback;
		float final_value;
	public:

		// Default action is nothing
		AxisAction();

		// Create a callable action for binding to joystick axes.
		// @param name Display name to help with searching and configuring
		// @param final_value Value sent to callback when this action is unbound
		// @param callback The action to run
		AxisAction(const std::string& name, float final_value = 0.0F, const decltype(callback)& callback = nullptr);
		
		inline const std::string& name() const {
			return display_name;
		}
		inline void call(float x) const {
			callback(x);
		}
		inline void disconnect() const {
			call(final_value);
		}
		inline void operator()(float x) const {
			call(x);
		}

		// Compare actions based on their names (alphabetical order)
		inline bool operator<(const AxisAction& other) const {
			return display_name < other.display_name;
		}
};


// Two-sided scaling: joysticks usually don't center exactly at 0, so scale around a calibrated center
// There are two separate scales since one side will cover a larger range of values
// Graph of raw input translation: https://www.desmos.com/calculator/gt7h3bbf2i
class AxisCalibration {
	private:
		float left_scale = 1.0F;
		float right_scale = 1.0F;
		float center_pos = 0.0F;
		float deadzone = 0.0F;
		float minimum = -1.0F;
		float maximum = 1.0F;
		void rescale();
	public:
		void restore_defaults();

		// Minimum position the joystick can reach (almost always -1.0)
		void set_min(float);
		inline float min() const { return minimum; }

		// Maximum position the joystick can reach (almost always 1.0)
		void set_max(float);
		inline float max() const { return maximum; }

		// Position where the joystick centers (default 0.0, but most joysticks are off by a bit)
		void set_center(float);
		inline float center() const { return center_pos; }

		// The dead-zone is a region around the center where input is ignored. Should be in range [0.0, 1.0] 
		void set_dead_zone(float percent);
		inline float dead_zone() const { return deadzone; }

		// Scale a raw input to the calibrated output
		float translate(float x) const;
};

class JoystickAxis {
	public:

		constexpr static float AXIS_MIN = -1.0F;
		constexpr static float AXIS_MAX = 1.0F;
		constexpr static float AXIS_RANGE = AXIS_MAX - AXIS_MIN;
		static float percent_to_axis(float x);
		static float axis_to_percent(float x);

		inline const AxisAction& action() const {
			return _action;
		}
		inline const AxisCalibration& calibration() const {
			return cal;
		}
		inline AxisCalibration& calibration() {
			return cal;
		}
		inline float value() const { 
			return last_value_translated;
		}
		inline float raw_value() const {
			return last_value_raw;
		}

		void set_action(const AxisAction& act);
		void update(float x);
		void unbind();

		void apply_calibration(const AxisCalibration&);
		// Begin calibrating. The axis should in default (at rest/released) position when starting
		void start_calibration();
		// Finish calibrating and restore the displaced action
		void end_calibration();

	private:
		
		AxisCalibration cal;

		float last_value_raw;
		float last_value_translated;

		AxisAction _action;
		AxisAction displaced_action;
};

// Constants for the gamepad mappings
namespace gamepad {
	// Axis details for a gamepad mapping.
	// Do not confuse with JoystickAxis. This is not the device, only const names/values
	struct Axis {
		const char* CODE_NAME;
		const char* DISPLAY_NAME;
		int IDX;
	};
	constexpr const Axis left_x {
		.CODE_NAME = "left_x",
		.DISPLAY_NAME = "Left Joystick X",
		.IDX = GLFW_GAMEPAD_AXIS_LEFT_X
	};

	constexpr const Axis left_y {
		.CODE_NAME = "left_y",
		.DISPLAY_NAME = "Left Joystick Y",
		.IDX = GLFW_GAMEPAD_AXIS_LEFT_Y
	};
	constexpr const Axis right_x {
		.CODE_NAME = "right_x",
		.DISPLAY_NAME = "Right Joystick X",
		.IDX = GLFW_GAMEPAD_AXIS_RIGHT_X
	};
	constexpr const Axis right_y {
		.CODE_NAME = "right_y",
		.DISPLAY_NAME = "Right Joystick Y",
		.IDX = GLFW_GAMEPAD_AXIS_RIGHT_Y
	};
	constexpr const Axis left_trigger {
		.CODE_NAME = "left_trigger",
		.DISPLAY_NAME = "Left Trigger",
		.IDX = GLFW_GAMEPAD_AXIS_LEFT_TRIGGER
	};
	constexpr const Axis right_trigger {
		.CODE_NAME = "right_trigger",
		.DISPLAY_NAME = "Right Trigger",
		.IDX = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER
	};
	constexpr std::array<const Axis, 6> AXES = {left_x, left_y, right_x, right_y, left_trigger, right_trigger};
}

/*
	One Controller/Gamepad device on the system

	Name and order of axes are system dependent, but if GLFW supports "Gamepad" mappings (1)
	for this device, then the gamepad axes order is used (2)

	1. https://www.glfw.org/docs/3.3/input_guide.html#gamepad
	2. https://www.glfw.org/docs/3.3/group__gamepad__axes.html
*/
class Controller {
	private:
		std::string name;
		std::vector<JoystickAxis> _axes;
		int _joystick_id = -1;
		bool _present = false;
		bool gamepad_mode = false;
	public:

		// Read hardware information for this device
		void update_device();

		// Read axis positions and call event handlers
		void update_axes();

		JoystickAxis& get_gamepad_axis(int glfw_gamepad_axis);
		
		inline JoystickAxis& get_gamepad_axis(const gamepad::Axis& a) {
			return get_gamepad_axis(a.IDX);
		}

		const char* device_name() const;
	
		inline void set_joystick_id(int id) {
			_joystick_id = id;
		}
		inline int joystick_id() const {
			return _joystick_id;
		}
		inline bool present() const {
			return _present;
		}
		inline decltype(_axes)& axes() {
			return _axes;
		}
		inline bool is_gamepad() const {
			return gamepad_mode;
		}
};