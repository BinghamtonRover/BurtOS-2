#include <iostream>

#include "drive_controller.hpp"

DriveController drive;

int main() {
    std::cout << "Binghamton University Rover Team - BurtOS 2 - Rover Subsystem v2\n";


    std::cout << "Initialization complete; Entering main event loop\n";
    for (;;) {
        drive.update_motor_acceleration();
    }

    return 0;   
}
