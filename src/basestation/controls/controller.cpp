#include "controller.hpp"

#include <GLFW/glfw3.h>
#include <limits>
#include <cmath>

float JoystickAxis::translate(float x) {
	if (x < center - dead_zone) {
		x = left_scale * (x - minimum) + AXIS_MIN;
	} else if (x > center + dead_zone) {
		x = right_scale * (x - center - dead_zone);
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
