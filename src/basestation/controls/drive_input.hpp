#pragma once

#include <rover_control.hpp>
#include <controls/controller_manager.hpp>

/*
	Provides event handlers for controller and keyboard and combines them into one output speed

	Derived from rc::Drive
*/
class DriveInput : public rc::Drive {
	public:
		DriveInput(net::MessageSender& to_subsystem);

		void poll_events();
		void add_controller_actions(ControllerManager&);

		float throttle() const { return controller_speed_cap; }
		void set_throttle(float t) { controller_speed_cap = t; }

		void set_turn_scale(float max_degrees) { controller_max_angle = max_degrees; }

	private:
		float controller_speed_cap = 2.0F;
		float controller_fwd_speed = 0.0F;
		float controller_rev_speed = 0.0F;
		float controller_angle = 0.0F;
		float controller_max_angle = 90.0F;

		float out_speed;
		float out_angle;

		void merge_input_sources();

};
