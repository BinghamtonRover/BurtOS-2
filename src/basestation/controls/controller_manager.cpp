#include "controller_manager.hpp"

#include <algorithm>

ControllerManager* ControllerManager::main_controller_manager = nullptr;

ControllerManager::ControllerManager() {
	for (unsigned int i = 0; i < _devices.size(); i++) {
		_devices[i].set_joystick_id(i);
	}
}

void ControllerManager::init() {
	main_controller_manager = this;
	glfwSetJoystickCallback(glfw_joystick_callback);
	for (Controller& c : _devices) {
		c.update_device();
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

const AxisAction& ControllerManager::find_action(const std::string& name) const {
	AxisAction search;
	search.name = name;
	auto it = std::lower_bound(_actions.begin(), _actions.end(), search);
	if (it != _actions.end() && it->name == name) {
		return *it;
	} else {
		throw std::invalid_argument("action not found");
	}
}

void ControllerManager::glfw_joystick_callback(int joystick_id, int event) {
	main_controller_manager->_devices[joystick_id].update_device();
	main_controller_manager->hw_cfg++;
}

void ControllerManager::add_axis_action(const std::string& name, const std::function<void(float)>& action_callback, float final_value) {
	AxisAction new_action;
	new_action.name = name;
	new_action.callback = action_callback;
	new_action.final_value = final_value;

	auto it = std::lower_bound(_actions.begin(), _actions.end(), new_action);
	if (it != _actions.end() && it->name == name) {
		// name was already added: replace it
		*it = new_action;
	} else {
		// name does not exist yet: insert in sorted position
		_actions.insert(std::upper_bound(_actions.begin(), _actions.end(), new_action), new_action);
	}
}
