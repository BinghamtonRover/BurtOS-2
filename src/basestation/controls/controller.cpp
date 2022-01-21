#include "controller.hpp"

#include <GLFW/glfw3.h>
#include <limits>
#include <cmath>
#include <iostream>

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

void Controller::update_device() {
	_present = glfwJoystickPresent(_joystick_id);
	if (_present) {
		std::cout << "present!!!\n";
		int axis_count;
		glfwGetJoystickAxes(_joystick_id, &axis_count);

		if (_axes.size() != axis_count) {
			_axes.resize(axis_count);
		}

	} else {
		std::cout << "not present :(\n";
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
	return glfwGetJoystickName(_joystick_id);
}