#pragma once

#include <iostream> 
#include <cmath>

class DriveController {
	private:
	public:
		float wheel_size = 15; // In cm 

		// accelerations for each motor
		float left_motor_1 = 0;
		float left_motor_2 = 0;
		float left_motor_3 = 0;

		float right_motor_1 = 0;
		float right_motor_2 = 0;
		float right_motor_3 = 0;


		float current_angle = 0;
		float target_velocity_magnitude = 0;

		float current_velocity_x = 0;
		float current_velocity_y = 0;

		float target_velocity_x = 0;
		float target_velocity_y = 0;

		// max acc = max acc for a specific motor 
		float MAX_ACCELERATION = 15;
		// max velocity for rover 
		float MAX_VELOCITY = 2000;

		void halt() {
			accelerate_to(0, 0);
		}

		void set_motor_acc(char direction, float acc) { 
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
		void update_target_velocity() { 
			target_velocity_y = cos(current_angle) * target_velocity_magnitude;
			target_velocity_x = cos(90 + current_angle) * target_velocity_magnitude;
		}

		float get_velocity() { 
			return sqrt(pow(current_velocity_x, 2) + (current_velocity_y, 2));
		}
		
		// moves to target velocities? has to find angle 
		void accelerate_to(float target_x, float target_y) { 
			
			float acceleration_x = target_x - current_velocity_x;
			float acceleration_y = target_y - current_velocity_y;

			float target_acceleration = sqrt(pow(acceleration_x, 2) + (acceleration_y, 2));
			float angle = atan(acceleration_y / acceleration_x);

			if (target_acceleration > 2) {     
				target_acceleration = 2;
				acceleration_y = cos(angle) * target_acceleration;
				acceleration_x = cos(90 + angle) * target_acceleration;
			}
			
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
		void update_motor_acceleration() {
			accelerate_to(target_velocity_x, target_velocity_y);
		}
		/* Tried to add something that was better than scaling linearly but this sometimes results in 
			negative values for motor accelerations 
		if (left_motor_1 > MAX_ACCELERATION) {
			left_motor_1 = acceleration_y - acceleration_x;
			left_motor_2 = acceleration_y - acceleration_x;
			left_motor_3 = acceleration_y - acceleration_x;
			
			right_motor_1 = acceleration_y + acceleration_x;
			right_motor_2 = acceleration_y + acceleration_x;
			right_motor_3 = acceleration_y + acceleration_x;
			// shluld be 1 when angle = 45, -1 when angle = -45 
			acceleration_y = MAX_ACCELERATION * cos((current_angle - 45) * 2 );
		}
		else if (acceleration_y + acceleration_x > MAX_ACCELERATION) {
			acceleration_x = MAX_ACCELERATION;
			// shluld be 1 when angle = 45, -1 when angle = -45 
			acceleration_y = MAX_ACCELERATION * cos((current_angle - 45) * 2 );
		}
		*/ 

		void setForwardVelocity(float mps) {
			// convert meters/second into revolutions per second
			target_velocity_magnitude = mps / (2 * 3.14159 * wheel_size / 100);
			update_target_velocity();
		} 
		
		
		// -90 = sharp left, 0 = straight, 90 = sharp right
		void setSteeringAngle(int8_t angle) { 
			current_angle = angle;
			update_target_velocity();
		}

};
