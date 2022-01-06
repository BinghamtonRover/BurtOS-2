#pragma once

#include <iostream> 
#include <cmath>
#include <math.h>

class DriveController {
	private:
		static constexpr float WHEEL_SIZE = 15; // In cm 

		// accelerations for each motor
		float left_motor_1 = 0;
		float left_motor_2 = 0;
		float left_motor_3 = 0;

		float right_motor_1 = 0;
		float right_motor_2 = 0;
		float right_motor_3 = 0;


		float target_velocity_rps = 0;
		float target_velocity_mps = 0;

		float target_angle = 0;
		float target_left_speed = 0;
		float target_right_speed = 0; 

		float left_speed = 0;
		float right_speed = 0;
		
		void set_motor_acc(char direction, float acc);
		void update_target_velocity();		
	public:
		void set_forward_velocity(float mps);
		void set_steering_angle(int8_t angle);
		void update_motor_acceleration();
		void halt();
		float get_velocity();
};
