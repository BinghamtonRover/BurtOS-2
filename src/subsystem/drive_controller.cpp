#include "drive_controller.hpp"

void DriveController::halt() {
    target_left_speed = 0;
    target_right_speed = 0;
    target_angle = 0;
    update_target_velocity();
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
    left_speed = target_left_speed;
    right_speed = target_right_speed; 
}

void DriveController::set_forward_velocity(float mps) {
    target_velocity_mps = mps;
    target_velocity_rps = (mps / 6.923) * M_PI * .271;
    update_target_velocity();
}

// -90 = sharp left, 0 = straight, 90 = sharp right
void DriveController::set_steering_angle(int8_t angle) { 
    target_angle = angle;
    update_target_velocity();
}
