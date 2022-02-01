#include <iostream>
#include <boost/asio.hpp>
#include <rover_system_messages.hpp>

#include <network.hpp>
#include "drive_controller.hpp"

DriveController drive_controller;
unsigned short int onboard_rover_receiver = 22101;

int main() {
	std::cout << "Binghamton University Rover Team - BurtOS 2 - Rover Subsystem v2\n";

	register_messages();

	static boost::asio::io_context ctx;
	static net::MessageReceiver receiver(onboard_rover_receiver, ctx);
	static net::MessageSender sender(ctx, net::Destination(boost::asio::ip::address::from_string("127.0.0.1"), onboard_rover_receiver));

	receiver.register_handler<drive_msg::Velocity>([](const uint8_t buf[], std::size_t len) {
		drive::Velocity msg;
		if (msg.ParseFromArray(buf, len)) {
			drive_controller.set_forward_velocity(msg.speed());
			drive_controller.set_steering_angle(msg.angle());
		}
	});

	receiver.register_handler<drive_msg::Halt>([](const uint8_t buf[], std::size_t len) {
		drive::Halt msg;
		if (msg.ParseFromArray(buf, len) && msg.halt()) {
			drive_controller.halt();
		}
	});

	receiver.register_handler<drive_msg::DriveMode>([](const uint8_t buf[], std::size_t len) {
		drive::DriveMode msg;
		if (msg.ParseFromArray(buf, len)) {
			DriveController::DriveMode mode = static_cast<DriveController::DriveMode>(msg.mode());
			drive_controller.set_drive_mode(mode);
		}
	});

	std::cout << "Initialization complete; Entering main event loop\n";
	for (;;) {
		ctx.poll();
		drive_controller.update_motor_acceleration();
	}

	return 0;
}
