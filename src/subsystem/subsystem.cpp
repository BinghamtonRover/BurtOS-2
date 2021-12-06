#include <iostream>
#include "drive_controller.hpp"

int main() {
	DriveController d = DriveController();
	d.setForwardVelocity(100);
	d.setSteeringAngle(45);
	// d.update_motor_acceleration();
	std::cout << "Wheel size: " << d.wheel_size << std::endl;
	std::cout << "Steering angle: " << d.current_angle << std::endl;
	std::cout << "Left motor acc: " << d.left_motor_1 << " " << d.left_motor_2 << " " << d.left_motor_3 << std::endl;
	std::cout << "Right motor acc: " << d.right_motor_1 << " " << d.right_motor_2 << " " << d.right_motor_3 << std::endl;

	DriveController negative_acc = DriveController();
	
	negative_acc.setForwardVelocity(-100);
	negative_acc.setSteeringAngle(45);
	std::cout << "Wheel size: " << negative_acc.wheel_size << std::endl;
	std::cout << "Steering angle: " << negative_acc.current_angle << std::endl;
	std::cout << "Left motor acc: " << negative_acc.left_motor_1 << " " << negative_acc.left_motor_2 << " " << negative_acc.left_motor_3 << std::endl;
	std::cout << "Right motor acc: " << negative_acc.right_motor_1 << " " << negative_acc.right_motor_2 << " " << negative_acc.right_motor_3 << std::endl;

	for(int i=0;i<10;i++) {
		d.update_motor_acceleration();
		std::cout << "Left motor acc: " << d.left_motor_1 << " " << d.left_motor_2 << " " << d.left_motor_3 << std::endl;
		std::cout << "Right motor acc: " << d.right_motor_1 << " " << d.right_motor_2 << " " << d.right_motor_3 << std::endl;
		std::cout << "velocities x:" << d.current_velocity_x << " y: " << d.current_velocity_y << std::endl;
	}
	return 0;
}
