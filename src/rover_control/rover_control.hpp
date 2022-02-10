#include <network.hpp>
#include <rover_system_messages.hpp>

namespace rc {

class Drive {
	public:
		Drive(net::MessageSender& ms);

		void set_interval(int milliseconds);
		int get_interval();
		void poll_events();

		void set_drive_mode(::drive::DriveMode_Mode mode);
		void halt();

		void set_speed(float speed);
		void set_angle(float angle);
		void set_movement(float speed, float angle);

		void register_listen_handlers(net::MessageReceiver& m);

	private:
		int interval = 100;
		net::MessageSender& sender;
		drive_msg::Velocity movement_message;

		std::chrono::steady_clock::time_point last_message_sent = std::chrono::steady_clock::now();

		float actual_left_speed;
		float actual_right_speed;
		::drive::DriveMode_Mode actual_drive_mode;
		std::chrono::steady_clock::time_point last_update_received;
};

} // namespace rc
