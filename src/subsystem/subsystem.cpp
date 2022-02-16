#include <iostream>
#include <csignal>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <rover_system_messages.hpp>

#include <network.hpp>
#include "drive_controller.hpp"
#include "can/rover_can.hpp"

DriveController drive_controller;
boost::asio::io_context ctx;

uint16_t subsystem_receive_port;

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

	//Send emergency stop messages to the odrives
	can_send(Node::DRIVE_AXIS_0, Command::DRIVE_EMERGENCY_STOP_MESSAGE, 0);
	can_send(Node::DRIVE_AXIS_1, Command::DRIVE_EMERGENCY_STOP_MESSAGE, 0);
	can_send(Node::DRIVE_AXIS_2, Command::DRIVE_EMERGENCY_STOP_MESSAGE, 0);
	can_send(Node::DRIVE_AXIS_3, Command::DRIVE_EMERGENCY_STOP_MESSAGE, 0);
	can_send(Node::DRIVE_AXIS_4, Command::DRIVE_EMERGENCY_STOP_MESSAGE, 0);
	can_send(Node::DRIVE_AXIS_5, Command::DRIVE_EMERGENCY_STOP_MESSAGE, 0);

	std::cerr << "Critical system shutdown finished. Terminating.\n";
	exit(1);
}

int main() {
	std::cout << "Binghamton University Rover Team - BurtOS 2 - Rover Subsystem v2\n";

	read_subsystem_config("cfg/subsystem_config.json");
	register_messages();

	net::MessageReceiver receiver(ctx, subsystem_receive_port);

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
