#pragma once

#include <vector>
#include <string>
#include <functional>

struct AxisAction {
	std::string name = "<unbound>";
	std::function<void(float)> callback;
	float final_value;

	// Compare actions based on their names (alphabetical order)
	inline bool operator<(const AxisAction& other) const {
		return name < other.name;
	}
};

class JoystickAxis {
	public:

		constexpr static float AXIS_MIN = -1.0F;
		constexpr static float AXIS_MAX = 1.0F;
		constexpr static float AXIS_RANGE = AXIS_MAX - AXIS_MIN;

		inline void update(float x) {
			last_value_raw = x;
			last_value_translated = translate(x);
			if (_action.callback) _action.callback(last_value_translated);
		}
		inline const AxisAction& action() const {
			return _action;
		}
		inline void set_action(const AxisAction& act) {
			_action = act;
		}
		inline float value() const { 
			return last_value_translated;
		}
		inline float raw_value() const {
			return last_value_raw;
		}

		// Scale the raw controller input to the calibrated range for this joystick
		float translate(float);
	private:
		// Joystick calibration:
		// Min and max show the range achievable by this axis. Typically all axes cover the entire [-1, 1]
		float minimum = AXIS_MIN;
		float maximum = AXIS_MAX;
		// Two-sided scaling: joysticks usually don't center exactly at 0, so scale around a calibrated center
		// There are two separate scales since one side will cover a larger range of values
		float left_scale = 1.0F;
		float right_scale = 1.0F;
		float center = 0.0F;
		float dead_zone = 0.0F;

		float last_value_raw;
		float last_value_translated;

		AxisAction _action;
};

class Controller {
	private:
		int _joystick_id = -1;
		bool _present = false;
		std::vector<JoystickAxis> _axes;
	public:

		void update_device();
		void update_axes();

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
};