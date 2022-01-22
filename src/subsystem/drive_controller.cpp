#include "drive_controller.hpp"
#include <stdio.h>

void DriveController::halt() {
    target_left_speed = 0;
    target_right_speed = 0;
    target_angle = 0;
}

void DriveController::set_motor_acc(char direction, float acc) { 
    if (direction == 'L') {
        left_motor_1 = acc;
        left_motor_2 = acc;
        left_motor_3 = acc;
    }
    if (direction == 'R') {
        right_motor_1 = acc;
        right_motor_2 = acc;
        right_motor_3 = acc;
    }
}

// Called whenever target_angle or target_velocity changes
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

float DriveController::get_velocity() { 
    return target_velocity_rps;
}

// Uses globals target_angle, acceleration to get motor accs
void DriveController::update_motor_acceleration() {
    if (last_acceleration_time == NULL) {
        last_acceleration_time = std::chrono::high_resolution_clock::now();
        return;
    }

    std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
    float time_difference = (current_time - last_acceleration_time).count();  
    
    float max_acceleration = ACCELERATION_LIMIT * fmax(OVER_ACCELERATION_LIMIT, time_difference);

    float difference_left = target_left_speed - left_speed;
    float difference_right = target_right_speed - right_speed;

    left_speed = right_speed + difference_right * time_difference; 
    right_speed = right_speed + difference_right * time_difference; 

    if (difference_left > 0) { 
        difference_left = fmin(difference_left, max_acceleration);    
    }
    else if (left_speed < 0 && difference_left < 0) { 
        difference_left = -fmin(-difference_left, max_acceleration);
    }
    
    if (difference_right > 0) { 
        difference_right = fmin(difference_right, max_acceleration);    
    }
    else if (left_speed < 0 && difference_right < 0) { 
        difference_left = -fmin(-difference_right, max_acceleration);
    }

    set_motor_acc('L', difference_left);
    set_motor_acc('R', difference_right);
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

/*std::ostream& operator << (std::ostream &os, const DriveController &s) {
    return os << "speeds (left, right): (" << s.left_speed << ", " << s.right_speed << ")";
}*/