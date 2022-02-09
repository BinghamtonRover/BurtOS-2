#pragma once

#include <cstdint>
#include <chrono>
#include "can/rover_can.hpp"

class DriveController {
	public:
		void set_forward_velocity(float mps);
		void set_steering_angle(float angle);

		// Update and apply target speeds and apply acceleration rate limits
		void update_motor_acceleration();
		void halt();
		float get_target_velocity();

		enum class DriveMode { NEUTRAL, DRIVE, COUNT };

		DriveMode get_drive_mode();
		void set_drive_mode(DriveMode mode);

		float target_left_speed = 0.0F;
		float target_right_speed = 0.0F;
 

	private:
		constexpr static float GEARBOX_RATIO = 6.923F;
		constexpr static float WHEEL_DIAMETER_METERS = 0.271F;
		constexpr static float MAX_SPEED = 6.1F;
		// This should be greater than 3.0
		constexpr static double ACCELERATION = 3.0;

		// Desired movement parameters for primary drive mode
		float target_velocity_mps = 0.0F;
		float target_velocity_rps = 0.0F;
		float target_angle = 0.0F;

		// Actual speeds to be sent to ODrives (revolutions per second)
		float left_speed = 0.0F;
		float right_speed = 0.0F;

		//Clock 
		const std::chrono::time_point<std::chrono::high_resolution_clock> time_now = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> time_updated =  std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> time_difference = time_now - time_updated;

		// Call whenever target_angle or target_velocity changes
		void update_target_velocity();		
		
		DriveMode current_mode = DriveMode::NEUTRAL;
};
