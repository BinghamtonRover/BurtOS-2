#include <iostream>
#include <csignal>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <rover_system_messages.hpp>

#include <network.hpp>
#include "drive_controller.hpp"

DriveController drive_controller;
boost::asio::io_context ctx;

uint16_t subsystem_receive_port;

std::chrono::steady_clock::time_point last_message_sent = std::chrono::steady_clock::now();;
int message_interval = 100; //milliseconds

void read_subsystem_config(const std::string& fname) {
	namespace ptree = boost::property_tree;
	ptree::ptree subsystem_cfg;
	try {
		ptree::json_parser::read_json(fname, subsystem_cfg);
		subsystem_receive_port = subsystem_cfg.get<uint16_t>("subsystem.network.recv_port", 22101);

	} catch (const ptree::json_parser_error& err) {
		std::cerr << "Warning: Using default config after error reading config file: "
				<< err.message() << " (" << err.filename() << ":" << err.line() << ")" << std::endl;
	}
}

// Try to deinitialize critical systems (like the ODrives) after the program has encountered a critical error
// Exit with abnormal exit code when finished
void panic_shutdown() {
	std::cerr << "Attempting to stop critical systems...\n";

	// TODO: (IMPORTANT) Send CAN commands to stop all subsystems
	// Unlike the peaceful shutdown, this should only shutdown critical systems (ODrives, other motor drivers)

	std::cerr << "Critical system shutdown finished. Terminating.\n";
	exit(1);
}

int main() {
	std::cout << "Binghamton University Rover Team - BurtOS 2 - Rover Subsystem v2\n";

	read_subsystem_config("cfg/subsystem_config.json");
	register_messages();

	net::MessageReceiver receiver(ctx, subsystem_receive_port);
	net::MessageSender sender(ctx, net::Destination(boost::asio::ip::address::from_string("127.0.0.1"), subsystem_receive_port));

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

	static bool operating = true;

	// Peacefully shut down when receiving SIGINT
	std::signal(SIGINT, [](int signal) {
		std::cout << "Disabling operations in response to SIGINT.\n";
		operating = false;
	});

	// Segmentation fault: as a last resort, at least try to deinitialize the subsystem
	std::signal(SIGSEGV, [](int signal) {
		std::cerr << "Fatal error: Segmentation fault!\n";
		panic_shutdown();
	});

	std::cout << "Initialization complete; Entering main event loop.\n";

	/*
		Main Event Loop
	*/

	try {
		while (operating) {

			ctx.poll();
			drive_controller.update_motor_acceleration();

			auto time_now = std::chrono::steady_clock::now();
			std::chrono::duration<double, std::milli> time_passed = time_now - last_message_sent;

			if (time_passed.count() >= message_interval) {
				drive_msg::ActualSpeed speed_message;
				speed_message.data.set_left(drive_controller.get_left_speed());
				speed_message.data.set_right(drive_controller.get_right_speed());
				sender.send_message(speed_message);

				drive_msg::DriveMode mode_message;
				::drive::DriveMode_Mode new_mode = static_cast<::drive::DriveMode_Mode>(drive_controller.get_drive_mode());
				mode_message.data.set_mode(new_mode);
				sender.send_message(mode_message);

				last_message_sent = std::chrono::steady_clock::now();
			}

		}
	} catch (const std::exception& err) {
		std::cerr << "Fatal error: Unhandled '" << typeid(err).name() << "' exception in Main Event Loop!\n\twhat(): " << err.what() << std::endl;
		panic_shutdown();
	}

	/*
		Peaceful Shutdown Procedure
	*/

	std::cout << "Subsystem shutting down...\n";
	receiver.close();
	ctx.stop();

	// TODO: (IMPORTANT) halt is not enough! Must also ensure speeds are sent to ODrives
	// This may be done by adding a destructor to drive controller instead
	drive_controller.halt();

	std::cout << "Operation complete.\n";

	return 0;
}
