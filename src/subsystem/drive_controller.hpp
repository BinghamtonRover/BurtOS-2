#pragma once

#include <iostream> 
#include <cmath>
#include <math.h>

class DriveController {
	private:
	public:
		static constexpr float WHEEL_SIZE = 15; // In cm 

		// accelerations for each motor
		float left_motor_1 = 0;
		float left_motor_2 = 0;
		float left_motor_3 = 0;

		float right_motor_1 = 0;
		float right_motor_2 = 0;
		float right_motor_3 = 0;

		float current_angle = 0;
		float target_velocity_mps = 0;
		float target_velocity_rps = 0;

		float current_velocity_x = 0;
		float current_velocity_y = 0;

		float target_velocity_x = 0;
		float target_velocity_y = 0;

		void halt();
		void set_motor_acc(char direction, float acc);
		void update_target_velocity();
		float get_velocity();
		void accelerate_to(float velocity_x, float velocity_y);
		void update_motor_acceleration();
		void set_forward_velocity(float mps);
		void set_steering_angle(int8_t angle);
};
