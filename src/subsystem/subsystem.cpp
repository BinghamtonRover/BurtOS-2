#include <iostream>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <rover_system_messages.hpp>

#include <network.hpp>
#include "drive_controller.hpp"

DriveController drive_controller;

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

int main() {
	std::cout << "Binghamton University Rover Team - BurtOS 2 - Rover Subsystem v2\n";

	read_subsystem_config("cfg/subsystem_config.json");
	register_messages();

	boost::asio::io_context ctx;
	net::MessageReceiver receiver(subsystem_receive_port, ctx);

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
