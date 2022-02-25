#pragma once

#include <cstdint>
#include <chrono>

class DriveController {
	public:
		void set_forward_velocity(float mps);
		void set_steering_angle(float angle);

		// Update and apply target speeds and apply acceleration rate limits
		void update_motor_acceleration();
		void update_motor_calibration();
		void halt();
		float get_target_velocity();

		float get_left_speed();
		float get_right_speed();

		enum class DriveMode { NEUTRAL, DRIVE, CALIBRATING, COUNT };

		DriveMode get_drive_mode();
		void set_drive_mode(DriveMode mode);

		static float to_rps(float mps);
		static float to_mps(float rps);

	private:
		constexpr static float GEARBOX_RATIO = 6.923F;
		constexpr static float WHEEL_DIAMETER_METERS = 0.271F;
		constexpr static float MAX_SPEED = 2.1F;
		constexpr static double CALIBRATION_TIME = 15.0;

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

		// Calibration stage (only when calibrating)
		int calibration_stage = -1;

		//Clock 
		std::chrono::time_point<std::chrono::steady_clock> time_can_updated{};

		// Call whenever target_angle or target_velocity changes
		void update_target_velocity();

		DriveMode current_mode = DriveMode::NEUTRAL;

		std::chrono::steady_clock::time_point last_active_time{};
};
