#include "controller.hpp"

#include <limits>
#include <cmath>
#include <stdexcept>

AxisAction::AxisAction(const std::string& name, float final_value, const decltype(callback)& callback)
		: display_name(name),
		callback(callback),
		final_value(final_value) {

	if (!callback) {
		this->callback = [](float) { };
	}

}

AxisAction::AxisAction() {
	display_name = "<none>";
	callback = [](float) { };
}

void AxisCalibration::set_min(float new_min) {
	minimum = new_min;
	rescale();
}

void AxisCalibration::set_max(float new_max) {
	maximum = new_max;
	rescale();
}

void AxisCalibration::set_center(float new_center) {
	if (new_center >= JoystickAxis::AXIS_MAX || new_center <= JoystickAxis::AXIS_MIN)
		throw std::invalid_argument("center must be within (-1,1)");

	center_pos = new_center;
	rescale();
}

void AxisCalibration::set_dead_zone(float new_deadzone) {
	if (new_deadzone < 0.0F || new_deadzone > 1.0F)
		throw std::invalid_argument("dead zone must be within [0,1]");

	deadzone = new_deadzone;
	rescale();
}

void AxisCalibration::rescale() {
	left_scale = (JoystickAxis::AXIS_RANGE / 2.0F) / ((center_pos - deadzone) - minimum);
	right_scale = (JoystickAxis::AXIS_RANGE / 2.0F) / (maximum - (center_pos + deadzone));
}

void AxisCalibration::restore_defaults() {
	left_scale = 1.0F;
	right_scale = 1.0F;
	center_pos = 0.0F;
	deadzone = 0.0F;
	minimum = -1.0F;
	maximum = 1.0F;
}

float AxisCalibration::translate(float x) const {
	if (x < center_pos - deadzone) {
		x = left_scale * (x - minimum) + JoystickAxis::AXIS_MIN;
	} else if (x > center_pos + deadzone) {
		x = right_scale * (x - center_pos - deadzone);
	} else {
		x = 0.0F;
	}

	if (x > JoystickAxis::AXIS_MAX) x = JoystickAxis::AXIS_MAX;
	else if (x < JoystickAxis::AXIS_MIN) x = JoystickAxis::AXIS_MIN;

	return x;
}

float JoystickAxis::axis_to_percent(float axis_value) {
	return (axis_value - JoystickAxis::AXIS_MIN) / JoystickAxis::AXIS_RANGE;
}

float JoystickAxis::percent_to_axis(float percent) {
	return percent * JoystickAxis::AXIS_RANGE + JoystickAxis::AXIS_MIN;
}

void JoystickAxis::update(float x) {
	last_value_raw = x;
	last_value_translated = cal.translate(x);
	_action(last_value_translated);
}

void JoystickAxis::set_action(const AxisAction& act) {
	_action.disconnect();
	_action = act;
}

void JoystickAxis::unbind() {
	_action.disconnect();
	_action = AxisAction();
}

void JoystickAxis::start_calibration() {
	displaced_action = _action;
	unbind();

	cal.set_min(raw_value());
	cal.set_max(raw_value());
	cal.set_center(raw_value());
	cal.set_dead_zone(0.0F);

	// During calibration, record the full range of motion
	// The final value is out of the natural range
	AxisAction calibrate_action("Calibrating", -1.1F, [this](float) {
		float x = raw_value();
		if (x < -1.0F) {
			// Ignore final value
		} else {
			cal.set_min(fmin(cal.min(), x));
			cal.set_max(fmax(cal.max(), x));
		}
	});
	set_action(calibrate_action);

}

void JoystickAxis::end_calibration() {

	// If the value recorded when calibration started (the resting value) is
	// equal to the min or max, then assume this axis doesn't rest in the center
	// Otherwise, assume it does rest centered and apply a small dead zone
	if (cal.center() == cal.min() || cal.center() == cal.max()) {
		cal.set_center(0.0F);
	} else {
		cal.set_dead_zone(0.08F);
	}

	set_action(displaced_action);
}

void JoystickAxis::apply_calibration(const AxisCalibration& c) {
	cal = c;
}

void Controller::update_device() {
	bool now_present = glfwJoystickPresent(_joystick_id);
	if (!now_present && _present) {
		// If the device was disconnected, propagate the final value to each axis
		for (auto& axis : _axes) {
			axis.action().disconnect();
		}
	}
	_present = now_present;
	if (_present) {
		name = glfwGetJoystickName(_joystick_id);

		int axis_count;
		glfwGetJoystickAxes(_joystick_id, &axis_count);

		// gamepad mode reorders axes by physical location rather than platform-specific index
		// feature is only available if GLFW/SDL_GameControllerDB has a "Gamepad mapping" for it
		gamepad_mode = glfwJoystickIsGamepad(_joystick_id);
		if (gamepad_mode) {
			// Allocate for gamepad axes rather than actual count
			axis_count = GLFW_GAMEPAD_AXIS_LAST + 1;
		}

		if (_axes.size() != axis_count) {
			// If the config has changed, unbind any old actions		
			for (auto& axis : _axes) {
				axis.unbind();
			}
			_axes.resize(axis_count);
		}
	}
}

void Controller::update_axes() {
	if (gamepad_mode) {
		GLFWgamepadstate state;
		if (glfwGetGamepadState(_joystick_id, &state)) {
			for (int i = 0; i <= GLFW_GAMEPAD_AXIS_LAST; i++) {
				_axes[i].update(state.axes[i]);
			}
		}
	} else {
		int count;
		const float* values = glfwGetJoystickAxes(_joystick_id, &count);
		while (count) {
			count--;
			_axes[count].update(values[count]);
		}
	}

}

const char* Controller::device_name() const {
	return name.c_str();
}

JoystickAxis& Controller::get_gamepad_axis(int glfw_gamepad_axis) {
	if (!_present || !gamepad_mode)
		throw std::runtime_error("Controller::get_gamepad_axis: gamepad mappings unavailable");
	
	if (glfw_gamepad_axis < 0 || glfw_gamepad_axis > GLFW_GAMEPAD_AXIS_LAST)
		throw std::invalid_argument("Controller::get_gamepad_axis: unknown axis");

	return _axes[glfw_gamepad_axis];
}
