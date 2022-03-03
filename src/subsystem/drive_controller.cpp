#include "drive_controller.hpp"

#include <cmath>
#include <boost/math/constants/constants.hpp>
#include <rover_can.hpp>

float DriveController::to_rps(float mps) {
	return (GEARBOX_RATIO * mps) / (boost::math::constants::pi<float>() * WHEEL_DIAMETER_METERS);
}

float DriveController::to_mps(float rps) {
	return (boost::math::constants::pi<float>() * WHEEL_DIAMETER_METERS * rps) / GEARBOX_RATIO;
}

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
	return to_mps(left_speed);
}

float DriveController::get_right_speed() {
	return to_mps(right_speed);
}

void DriveController::update_motor_acceleration() {
	//completely skip motor acceleration when calibrating
	if (current_mode == DriveMode::CALIBRATING) {
		update_motor_calibration();
		return;
	}

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
	can_send(Node::DRIVE_AXIS_5, Command::SET_INPUT_VEL, -left_speed);
	can_send(Node::DRIVE_AXIS_4, Command::SET_INPUT_VEL, right_speed);
	can_send(Node::DRIVE_AXIS_3, Command::SET_INPUT_VEL, -left_speed);
	can_send(Node::DRIVE_AXIS_2, Command::SET_INPUT_VEL, right_speed);
	can_send(Node::DRIVE_AXIS_1, Command::SET_INPUT_VEL, -left_speed);
	can_send(Node::DRIVE_AXIS_0, Command::SET_INPUT_VEL, right_speed);

	time_can_updated = std::chrono::steady_clock::now();
}

void DriveController::set_forward_velocity(float mps) {
	last_active_time = std::chrono::steady_clock::now();
	mps = fmin(fmax(mps, -MAX_SPEED), MAX_SPEED);
	target_velocity_mps = mps;
	target_velocity_rps = to_rps(mps);
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
	//Make sure mode is valid, and cannot change mode in the middle of calibrating
	if (mode < DriveMode::COUNT && current_mode != DriveMode::CALIBRATING) {
		last_active_time = std::chrono::steady_clock::now();
		current_mode = mode;
		update_motor_acceleration();
	}
}

//Initialize the drive (CAN bus initialization commands)
void DriveController::update_motor_calibration() {
	auto time_now = std::chrono::steady_clock::now();
	if (calibration_stage == -1) { time_can_updated = time_now; }
	std::chrono::duration<double> time_difference = time_now - time_can_updated;

	if (calibration_stage < (int)(time_difference.count() / CALIBRATION_TIME)) {
		calibration_stage++;
		//Start calibration sequence
		if (calibration_stage <= 5) {
			can_send(static_cast<Node>(calibration_stage + 1), Command::SET_AXIS_REQUESTED_STATE, 3); //START INIT SEQUENCE
		}

		//Set motors settings of previously calibrated motor
		if (calibration_stage > 0) {
			can_send(static_cast<Node>(calibration_stage), Command::SET_AXIS_REQUESTED_STATE, 8); //AXIS_STATE_CLOSED_LOOP_CONTROL
			can_send(static_cast<Node>(calibration_stage), Command::SET_CONTROLLER_MODES, 2);     //CONTROL_MODE_VELOCITY_CONTROL
		}
	}

	if (calibration_stage == 6) {
		current_mode = DriveMode::NEUTRAL;
		time_can_updated = std::chrono::steady_clock::now();
		calibration_stage = -1;
		last_active_time = std::chrono::steady_clock::now();
	}
}
