#include <controls/drive_input.hpp>

DriveInput::DriveInput(net::MessageSender& to_subsystem)
	: rc::Drive(to_subsystem) {

}

void DriveInput::poll_events() {
	if (update_ready()) {
		merge_input_sources();
		set_movement(out_speed, out_angle);
		send_update();
	}
}

void DriveInput::merge_input_sources() {
	float controller_speed = std::max(controller_fwd_speed, controller_rev_speed);
	if (controller_rev_speed > controller_fwd_speed)
		controller_speed = -controller_speed;

	// Future: This is where keyboard input will be compared with controller inputs
	out_speed = controller_speed;
	out_angle = controller_angle;

}

void DriveInput::add_controller_actions(ControllerManager& m) {
	m.add_axis_action("Accelerate", [this](float x) {
		controller_fwd_speed = JoystickAxis::axis_to_percent(x) * controller_speed_cap;
	}, -1.0F);

	m.add_axis_action("Reverse", [this](float x) {
		controller_rev_speed = JoystickAxis::axis_to_percent(x) * controller_speed_cap;
	}, -1.0F);

	m.add_axis_action("Steer", [this](float x) {
		controller_angle = x * controller_max_angle;
	}, 0.0F);
}
