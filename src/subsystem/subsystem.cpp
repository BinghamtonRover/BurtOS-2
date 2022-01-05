#include <iostream>
#include "drive_controller.cpp"
using namespace std;

int main() { 
    DriveController drive = DriveController();
    drive.current_angle = 10; 
    drive.set_forward_velocity(10);
    cout << drive.current_velocity_x << drive.current_velocity_y << endl;
    for(int i=0;i<2;i++) {
        drive.update_motor_acceleration();
        cout << drive << endl;
    }
    
    DriveController drive_negative_angle = DriveController();
    drive_negative_angle.current_angle = -10; 
    drive_negative_angle.set_forward_velocity(10);

    drive_negative_angle.update_motor_acceleration();
    cout << drive_negative_angle << endl;
    drive_negative_angle.halt();
    cout << drive_negative_angle << endl;
}