#pragma once

#include <cstdint>

class DriveController {
	private:

		// Desired movement parameters for primary drive mode
		float target_velocity_mps = 0.0F;
		float target_velocity_rps = 0.0F;
		float target_angle = 0.0F;

		// Goal speeds to reach safely
		float target_left_speed = 0.0F;
		float target_right_speed = 0.0F; 

		// Actual speeds to be sent to ODrives (revolutions per second)
		float left_speed = 0.0F;
		float right_speed = 0.0F;

		// Call whenever target_angle or target_velocity changes
		void update_target_velocity();		
	public:
		void set_forward_velocity(float mps);
		void set_steering_angle(int8_t angle);
		
		// Update and apply target speeds and apply acceleration rate limits
		void update_motor_acceleration();
		void halt();
		float get_target_velocity();
};