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
	//set current time
	time_now = std::chrono::system_clock::now();
	time_difference = time_now - time_updated();

	if(time_difference.count() < .002){
			return;
	}
	else{
		if (false && get_drive_mode() == DriveMode::NEUTRAL) {
			left_speed = 0;
			right_speed = 0;
			time_updated = std::chrono::system_clock::now();
		} else {
			if(target_left_speed == left_speed && target_right_speed == right_speed){
				//speed set, return
				return;
			}
			else{
				float last_left_speed = left_speed;
				float last_right_speed = right_speed;
				double delta_time = time.difference.count();
				//what should max time be if program stalls?
				if(delta_time > 0.01){ delta_time = 0.01;}

				if (left_speed > target_left_speed) { left_speed = left_speed - delta_time; }
				else if (left_speed < target_left_speed) { left_speed = left_speed + delta_time; }
				if (right_speed > target_right_speed) { right_speed = right_speed - delta_time; }
				else if (right_speed < target_right_speed) { right_speed = right_speed + delta_time; }

				time_updated = std::chrono::system_clock::now();

				//target speed reached (bandaid fix?)
				//should 0.01f be based off of max change in velocity? (max delta time * acceleration) + buffer?
				if (!(left_speed - target_left_speed < 0.01f && left_speed - last_left_speed < 0.01f)) {
					if ((last_left_speed > target_left_speed && left_speed < target_left_speed) ||
						(last_left_speed < target_left_speed && left_speed > target_left_speed)) { left_speed = target_left_speed; }
				}
				if (!(right_speed - target_right_speed < 0.01f && right_speed - last_right_speed < 0.01f)) {
					if ((last_right_speed > target_right_speed && right_speed < target_left_speed) ||
						(last_right_speed < target_right_speed && right_speed > target_left_speed)) { right_speed = target_right_speed; }
				}
			}
		}
		can_send(Node::DRIVE_AXIS_5, Command::SET_INPUT_VEL, right_speed);
		can_send(Node::DRIVE_AXIS_4, Command::SET_INPUT_VEL, left_speed);
	}
}

void DriveController::set_forward_velocity(float mps) {
	mps = fmin(fmax(mps, -MAX_SPEED), MAX_SPEED);
	target_velocity_mps = mps;
	// M_PI is technically non-standard C++
	// When C++20 support becomes ubiquitous, replace Boost with std::numbers::pi_v<float>
	target_velocity_rps = (GEARBOX_RATIO * mps) / (boost::math::constants::pi<float>() * WHEEL_DIAMETER_METERS);
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
	if (mode < DriveMode::COUNT) {
		current_mode = mode;
		update_motor_acceleration();
	}
}
