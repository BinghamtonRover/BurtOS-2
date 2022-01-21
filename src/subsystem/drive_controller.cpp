#include "drive_controller.hpp"

#include <cmath>

void DriveController::halt() {
	target_left_speed = 0;
	target_right_speed = 0;
	target_angle = 0;
	update_target_velocity();
	set_drive_mode(NEUTRAL);
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
	if (get_drive_mode() == NEUTRAL) {
		left_speed = 0;
		right_speed = 0;
	} else {
		left_speed = target_left_speed;
		right_speed = target_right_speed;
	}
}

void DriveController::set_forward_velocity(float mps) {
	mps = fmin(fmax(mps, -MAX_SPEED), MAX_SPEED);
	target_velocity_mps = mps;
	target_velocity_rps = (GEARBOX_RATIO * mps) / (static_cast<float>(M_PI) * WHEEL_DIAMETER_METERS);
	update_target_velocity();
}

// -90 = sharp left, 0 = straight, 90 = sharp right
void DriveController::set_steering_angle(float angle) { 
	target_angle = fmin(fmax(angle, -180), 180);
	update_target_velocity();
}

DriveController::DriveMode DriveController::get_drive_mode() {
	return current_mode;
}

void DriveController::set_drive_mode(DriveMode mode) {
	if (mode < COUNT) {
		current_mode = mode;
		update_motor_acceleration();
	}
}
