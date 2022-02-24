#include <network.hpp>
#include <rover_system_messages.hpp>
#include <events.hpp>

namespace rc {

class Drive {
	public:
		Drive(net::MessageSender& ms);

		void set_interval(int milliseconds);
		int get_interval();
		void poll_events();
		bool update_ready();
		void send_update();

		void set_drive_mode(::drive::DriveMode_Mode mode);
		void halt();

		void set_speed(float speed);
		void set_angle(float angle);
		void set_movement(float speed, float angle);

		void register_listen_handlers(net::MessageReceiver& m);

		float get_actual_left_speed();
		float get_actual_right_speed();
		::drive::DriveMode_Mode get_actual_drive_mode();
		inline const std::chrono::steady_clock::time_point& get_last_update_received() const { return last_update_received; }

		// public events
		event::Emitter<drive::DriveMode_Mode> EVENT_DRIVEMODE;
		event::Emitter<float, float> EVENT_SPEED;

	private:
		int interval = 100;
		net::MessageSender& sender;
		drive_msg::Velocity movement_message;

		std::chrono::steady_clock::time_point last_message_sent{};

		float actual_left_speed = 0.0F;
		float actual_right_speed = 0.0F;
		::drive::DriveMode_Mode actual_drive_mode = drive::DriveMode_Mode::DriveMode_Mode_NEUTRAL;
		drive::DriveMode_Mode requested_drive_mode = drive::DriveMode_Mode::DriveMode_Mode_NEUTRAL;
		
		std::chrono::steady_clock::time_point last_update_received{};
};

class Control {
	public:
		Control(net::MessageSender& ms);

		void register_listen_handlers(net::MessageReceiver& m);

		float get_ps_batt();
		float get_main_curr();
		float get_ps12_volt();
		float get_ps12_curr();
		float get_temp12();
		float get_ps5_volt();
		float get_ps5_curr();
		float get_temp5();
		float get_odrv0_curr();
		float get_odrv1_curr();
		float get_odrv2_curr();

		inline const std::chrono::steady_clock::time_point& get_last_update_received() const { return last_update_received; }

		event::Emitter<float, float> EVENT_MAIN_CONTROL;
		event::Emitter<float, float, float> EVENT_PS12_CONTROL;
		event::Emitter<float, float, float> EVENT_PS5_CONTROL;
		event::Emitter<float, float, float> EVENT_ODRV_CONTROL;

	private:
		net::MessageSender& sender;

		std::chrono::steady_clock::time_point last_update_received{};

		float ps_batt = 0.0F;
		float main_curr = 0.0F;
		float ps12_volt = 0.0F;
		float ps12_curr = 0.0F;
		float temp12 = 0.0F;
		float ps5_volt = 0.0F;
		float ps5_curr = 0.0F;
		float temp5 = 0.0F;
		float odrv0_curr = 0.0F;
		float odrv1_curr = 0.0F;
		float odrv2_curr = 0.0F;
};

} // namespace rc
