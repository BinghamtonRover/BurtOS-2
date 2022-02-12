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

float DriveController::get_left_speed() {
	return left_speed;
}

float DriveController::get_right_speed() {
	return right_speed;
}

void DriveController::update_motor_acceleration() {
	auto time_now = std::chrono::steady_clock::now();

	std::chrono::duration<double> inactive_time = time_now - last_active_time;
	if (inactive_time.count() > 1) {
		set_forward_velocity(0.0F);
	}

	std::chrono::duration<double> time_difference = time_now - time_can_updated;

	if (time_difference.count() < .002 || target_left_speed == left_speed && target_right_speed == right_speed) {
		return;
	} else if (current_mode == DriveMode::NEUTRAL) {
		left_speed = 0;
		right_speed = 0;
	} else {
		left_speed = target_left_speed;
		right_speed = target_right_speed;
	}
	can_send(Node::DRIVE_AXIS_5, Command::SET_INPUT_VEL, right_speed);
	can_send(Node::DRIVE_AXIS_4, Command::SET_INPUT_VEL, left_speed);
	can_send(Node::DRIVE_AXIS_3, Command::SET_INPUT_VEL, right_speed);
	can_send(Node::DRIVE_AXIS_2, Command::SET_INPUT_VEL, left_speed);
	can_send(Node::DRIVE_AXIS_1, Command::SET_INPUT_VEL, right_speed);
	can_send(Node::DRIVE_AXIS_0, Command::SET_INPUT_VEL, left_speed);

	time_can_updated = std::chrono::steady_clock::now();
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
