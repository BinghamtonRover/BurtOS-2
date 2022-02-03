#include <network.hpp>
#include "drive_controller.hpp"

namespace rc {

class Drive {
	public:
		Drive(net::MessageSender& ms);

        void set_interval(int milliseconds);
        int get_interval();
        void poll_events();

        void set_drive_mode(DriveController::DriveMode mode);
        void halt();

        void set_speed(float speed);
        void set_angle(float angle);
        void set_movement(float speed, float angle);

    private:
        int interval = 100;
		net::MessageSender sender;
		drive_msg::Velocity movement_message;

		std::chrono::steady_clock::time_point last_message_sent = std::chrono::steady_clock::now();
}
