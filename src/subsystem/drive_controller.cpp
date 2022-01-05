#include "drive_controller.hpp"
#include <stdio.h>
using namespace std;

// Separate the implementation from drive_controller.hpp and place here

void DriveController::halt() {
    accelerate_to(0, 0);
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

// Called whenever current_angle and target_velocity changes
void DriveController::update_target_velocity() { 
    target_velocity_y = cos(current_angle * M_PI / 180) * target_velocity_mps;
    target_velocity_x = sin(current_angle * M_PI / 180) * target_velocity_mps;
}

float DriveController::get_velocity() { 
    return sqrt(pow(current_velocity_x, 2) + pow(current_velocity_y, 2));
}

// moves to target velocities? has to find angle 
void DriveController::accelerate_to(float velocity_x, float velocity_y) {     
    float acceleration_x = velocity_x - current_velocity_x;
    float acceleration_y = velocity_y - current_velocity_y;
    float target_acceleration = sqrt(pow(acceleration_x, 2) + pow(acceleration_y, 2));
    float angle = atan(acceleration_y / acceleration_x) * (180 / M_PI);
    
    /*
    std::cout << angle << std::endl;
    std::cout << acceleration_x << " " << acceleration_y << std::endl;
    std::cout << get_velocity() << std::endl;
    */ 

    if (acceleration_x != 0 || acceleration_y != 0) {
        float right_scaling = (90 + angle)/180;
        set_motor_acc('L', (1 - right_scaling) * target_acceleration);
        set_motor_acc('R', right_scaling * target_acceleration);
        
        current_velocity_x += acceleration_x;
        current_velocity_y += acceleration_y;
    }
    else { 
        set_motor_acc('L', 0);
        set_motor_acc('R', 0);
    }
}

// Uses globals current_angle, acceleration to get motor accs
void DriveController::update_motor_acceleration() {
    accelerate_to(target_velocity_x, target_velocity_y);
}

void DriveController::set_forward_velocity(float mps) {
    // convert meters/second into revolutions per second
    target_velocity_mps = mps;
    target_velocity_rps = mps / (2 * 3.14159 * WHEEL_SIZE / 100);
    update_target_velocity();
} 

// -90 = sharp left, 0 = straight, 90 = sharp right
void DriveController::set_steering_angle(int8_t angle) { 
    current_angle = angle;
    update_target_velocity();
}

ostream& operator << (ostream &os, const DriveController &s) {
    return os << "velocities (x, y): (" << s.current_velocity_x << ", " << s.current_velocity_y << ")";
}

int main() { 
    DriveController drive = DriveController();
    drive.current_angle = 10; 
    drive.set_forward_velocity(10);
    cout << drive.current_velocity_x << drive.current_velocity_y << endl;
    for(int i=0;i<2;i++) {
        drive.update_motor_acceleration();
        cout << drive << endl;
        //cout << "velocities (x, y): (" << drive.current_velocity_x << ", " << drive.current_velocity_y << ")" << endl;
    }
    
    DriveController drive_negative_angle = DriveController();
    drive_negative_angle.current_angle = -10; 
    drive_negative_angle.set_forward_velocity(10);

    drive_negative_angle.update_motor_acceleration();
    cout << drive_negative_angle << endl;
    drive_negative_angle.halt();
    cout << drive_negative_angle << endl;

}