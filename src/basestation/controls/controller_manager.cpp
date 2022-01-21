#include "controller_manager.hpp"

#include <algorithm>
#include <iostream>

ControllerManager::ControllerManager() {
	for (unsigned int i = 0; i < _devices.size(); i++) {
		_devices[i].set_joystick_id(i);
		glfwSetJoystickUserPointer(i, this);

		_devices[i].update_device();
	}
	glfwSetJoystickCallback(glfw_joystick_callback);
}

ControllerManager::~ControllerManager() {
	for (Controller& c : _devices) {
		if (glfwGetJoystickUserPointer(c.joystick_id()) == this) {
			glfwSetJoystickUserPointer(c.joystick_id(), nullptr);
		}
	}
}

void ControllerManager::scan_devices() {
	for (Controller& c : _devices) {
		c.update_device();
	}
}

void ControllerManager::update_controls() {
	for (Controller& c : _devices) {
		if (c.present()) {
			c.update_axes();
		}
	}
}

void ControllerManager::glfw_joystick_callback(int joystick_id, int event) {
	std::cout << "js event: " << joystick_id << " \n";
	ControllerManager* self = reinterpret_cast<ControllerManager*>(glfwGetJoystickUserPointer(joystick_id));
	if (self)
		self->_devices[joystick_id].update_device();
}

void ControllerManager::add_axis_action(const std::string& name, const std::function<void(float)>& action_callback, float final_value) {
	AxisAction new_action;
	new_action.name = name;
	new_action.callback = action_callback;
	new_action.final_value = final_value;

	auto it = std::lower_bound(actions.begin(), actions.end(), new_action);
	if (it != actions.end() && it->name == name) {
		// name was already added: replace it
		*it = new_action;
	} else {
		// name does not exist yet: insert in sorted position
		actions.insert(std::upper_bound(actions.begin(), actions.end(), new_action), new_action);
	}
}
