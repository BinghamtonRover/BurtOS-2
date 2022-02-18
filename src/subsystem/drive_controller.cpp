#include "drive_controller.hpp"

#include <cmath>
#include <boost/math/constants/constants.hpp>

void DriveController::halt() {
	target_velocity_mps = 0;
	target_velocity_rps = 0;
	target_angle = 0;
	update_target_velocity();
	set_drive_mode(DriveMode::NEUTRAL);
}

void DriveController::update_target_velocity() { 
	target_left_speed = target_velocity_rps; 
	target_right_speed = target_velocity_rps; 

	if (target_angle < 0) { 
		target_left_speed = target_velocity_rps * (90 + target_angle) / 90;
	}
	else if (target_angle > 0) { 
		target_right_speed = target_velocity_rps * (90 - target_angle) / 90;
	}
}

float DriveController::get_target_velocity() { 
	return target_velocity_mps;
}

void DriveController::update_motor_acceleration() {
	auto time_now = std::chrono::steady_clock::now();
	std::chrono::duration<double> time_passed = time_now - last_active_time;
	if (time_passed.count() > 1) {
		set_forward_velocity(0.0F);
	} else {
		if (get_drive_mode() == DriveMode::NEUTRAL) {
			left_speed = 0;
			right_speed = 0;
		} else {
			left_speed = target_left_speed;
			right_speed = target_right_speed;
		}
	}
}

void DriveController::set_forward_velocity(float mps) {
	last_active_time = std::chrono::steady_clock::now();
	mps = fmin(fmax(mps, -MAX_SPEED), MAX_SPEED);
	target_velocity_mps = mps;
	// M_PI is technically non-standard C++
	// When C++20 support becomes ubiquitous, replace Boost with std::numbers::pi_v<float>
	target_velocity_rps = (GEARBOX_RATIO * mps) / (boost::math::constants::pi<float>() * WHEEL_DIAMETER_METERS);
	update_target_velocity();
}

// -90 = sharp left, 0 = straight, 90 = sharp right
void DriveController::set_steering_angle(float angle) { 
	last_active_time = std::chrono::steady_clock::now();
	target_angle = fmin(fmax(angle, -180), 180);
	update_target_velocity();
}

DriveController::DriveMode DriveController::get_drive_mode() {
	return current_mode;
}

void DriveController::set_drive_mode(DriveMode mode) {
	if (mode < DriveMode::COUNT) {
		last_active_time = std::chrono::steady_clock::now();
		current_mode = mode;
		update_motor_acceleration();
	}
}

//Initialize the drive (CAN bus initialization commands)
bool DriveController::drive_init() {
	for (int i = static_cast<int>(Node::DRIVE_AXIS_0); i <= static_cast<int>(Node::DRIVE_AXIS_5); i++) {
#ifdef ONBOARD_CAN_BUS
		can_send((static_cast<Node>(i), Command::SET_AXIS_REQUESTED_STATE, 3); //START INIT SEQUENCE
#endif
		std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
		std::chrono::duration<double> delta_time = start_time - start_time;
		int tick = 0;

		while (delta_time.count() <= 30.0 && !can_check_hearbeat(static_cast<Node>(i))) { 
        	delta_time = std::chrono::steady_clock::now() - start_time;
		}

		//Initialization failed
		if (delta_time.count() > 30.0) { return false; }

		//Set motors settings
#ifdef ONBOARD_CAN_BUS
		can_send(static_cast<Node>(i), Command::SET_AXIS_REQUESTED_STATE, 8); //AXIS_STATE_CLOSED_LOOP_CONTROL
		can_send(static_cast<Node>(i), Command::SET_CONTROLLER_MODES, 2);     //CONTROL_MODE_VELOCITY_CONTROL
#endif
	}

    //Successfully initialized
	return true;
}
