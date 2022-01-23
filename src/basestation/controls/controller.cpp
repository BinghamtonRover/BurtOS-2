#include "controller.hpp"

#include <GLFW/glfw3.h>
#include <limits>
#include <cmath>
#include <stdexcept>

void AxisCalibration::set_center(float new_center) {
	if (center >= JoystickAxis::AXIS_MAX || center <= JoystickAxis::AXIS_MIN)
		throw std::invalid_argument("center must be within (-1,1)");

	center = new_center;
	rescale();
}

void AxisCalibration::set_dead_zone(float new_deadzone) {
	if (dead_zone < 0.0F || dead_zone > 1.0F)
		throw std::invalid_argument("dead zone must be within [0,1]");

	dead_zone = new_deadzone;
	rescale();
}

void AxisCalibration::rescale() {
	left_scale = (JoystickAxis::AXIS_RANGE / 2.0F) / ((center - dead_zone) - min);
	right_scale = (JoystickAxis::AXIS_RANGE / 2.0F) / (max - (center + dead_zone));
}

void AxisCalibration::restore_defaults() {
	left_scale = 1.0F;
	right_scale = 1.0F;
	center = 0.0F;
	dead_zone = 0.0F;
	min = -1.0F;
	max = 1.0F;
}

float JoystickAxis::translate(float x) const {
	if (x < cal.center - cal.dead_zone) {
		x = cal.left_scale * (x - cal.min) + AXIS_MIN;
	} else if (x > cal.center + cal.dead_zone) {
		x = cal.right_scale * (x - cal.center - cal.dead_zone);
	} else {
		x = 0.0F;
	}

	//return scale * (x - minimum) + AXIS_MIN;
	if (x > AXIS_MAX) x = AXIS_MAX;
	else if (x < AXIS_MIN) x = AXIS_MIN;

	return x;
}

void JoystickAxis::update(float x) {
	last_value_raw = x;
	last_value_translated = translate(x);
	if (_action.callback) _action.callback(last_value_translated);
}

void JoystickAxis::set_action(const AxisAction& act) {
	if (_action.callback)
		_action.callback(_action.final_value);
	_action = act;
}

void JoystickAxis::unbind() {
	if (_action.callback) {
		_action.callback(_action.final_value);
	}
	_action.name = AxisAction::UNBOUND_NAME;
	_action.callback = nullptr;
}

void JoystickAxis::start_calibration(AxisCalibrator& c) {
	unbind();
	c.displaced_action = _action;
	c.min = std::numeric_limits<float>::infinity();
	c.max = - std::numeric_limits<float>::infinity();

	AxisAction calibrate_action {
		.name = "Calibrating",
		.callback = [this, &c](float) {
			float x = raw_value();
			if (x < -1.0F) {
				// Received final value
				c.calibration_active = false;
				c.rescale();
			} else {
				c.min = fmin(c.min, x);
				c.max = fmax(c.max, x);
			}
		},
		.final_value = -1.1F
	};
	c.calibration_active = true;
	set_action(calibrate_action);

}

void JoystickAxis::end_calibration(AxisCalibrator& c) {
	if (!c.active())
		throw std::invalid_argument("not calibrating");
	set_action(c.displaced_action);
	c.calibration_active = false;
}

void JoystickAxis::apply_calibration(const AxisCalibration& c) {
	cal = c;
}

void Controller::update_device() {
	bool now_present = glfwJoystickPresent(_joystick_id);
	if (!now_present && _present) {
		// If the device was disconnected, propagate the final value to each axis
		for (auto& axis : _axes) {
			if (axis.action().callback) {
				axis.action().callback(axis.action().final_value);
			}
		}
	}
	_present = now_present;
	if (_present) {
		name = glfwGetJoystickName(_joystick_id);

		int axis_count;
		glfwGetJoystickAxes(_joystick_id, &axis_count);
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
	int count;
	const float* values = glfwGetJoystickAxes(_joystick_id, &count);
	if (count != _axes.size()) {
		update_device();
	}

	while (count) {
		count--;
		_axes[count].update(values[count]);
	}
}

const char* Controller::device_name() const {
	return name.c_str();
}
