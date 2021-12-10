#include "drive_controller.hpp"
#include <stdio.h>
using namespace std;

// Separate the implementation from drive_controller.hpp and place here
int main() { 
    cout << "hi" << endl;
    DriveController drive = DriveController();
    drive.current_angle = 10; 
    drive.setForwardVelocity(10);
    cout << drive.current_velocity_x << drive.current_velocity_y << endl;
    for(int i=0;i<2;i++) {
        drive.update_motor_acceleration();
        cout << "velocities (x, y): (" << drive.current_velocity_x << ", " << drive.current_velocity_y << ")" << endl;
    }
    
    DriveController drive_negative_angle = DriveController();
    drive_negative_angle.current_angle = -10; 
    drive_negative_angle.setForwardVelocity(10);
    for(int i=0;i<2;i++) {
        drive_negative_angle.update_motor_acceleration();
        cout << "velocities (x, y): (" << drive_negative_angle.current_velocity_x << ", " << drive_negative_angle.current_velocity_y << ")" << endl;
    }
}