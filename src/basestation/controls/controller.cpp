#include "controller.hpp"

#include <GLFW/glfw3.h>
#include <limits>
#include <cmath>
#include <stdexcept>

void AxisCalibration::set_center(float new_center) {
	if (new_center >= JoystickAxis::AXIS_MAX || new_center <= JoystickAxis::AXIS_MIN)
		throw std::invalid_argument("center must be within (-1,1)");

	center = new_center;
	rescale();
}

void AxisCalibration::set_dead_zone(float new_deadzone) {
	if (new_deadzone < 0.0F || new_deadzone > 1.0F)
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

void JoystickAxis::start_calibration() {
	displaced_action = _action;
	unbind();

	cal.min = raw_value();
	cal.max = raw_value();
	cal.center = raw_value();
	cal.dead_zone = 0.0F;

	// During calibration, record the full range of motion
	// The final value is out of the natural range
	AxisAction calibrate_action {
		.name = "Calibrating",
		.callback = [this](float) {
			float x = raw_value();
			if (x < -1.0F) {
				// Ignore final value
			} else {
				cal.min = fmin(cal.min, x);
				cal.max = fmax(cal.max, x);
			}
		},
		.final_value = -1.1F
	};
	set_action(calibrate_action);

}

void JoystickAxis::end_calibration() {

	// If the value recorded when calibration started (the resting value) is
	// equal to the min or max, then assume this axis doesn't rest in the center
	// Otherwise, assume it does rest centered and apply a small dead zone
	if (cal.center == cal.min || cal.center == cal.max) {
		cal.center = 0.0F;
	} else {
		cal.dead_zone = 0.08F;
	}
	cal.rescale();

	set_action(displaced_action);
}

void JoystickAxis::apply_calibration(const AxisCalibration& c) {
	cal = c;
	cal.rescale();
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
