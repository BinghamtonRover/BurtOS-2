#include <iostream>
#include <csignal>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <rover_system_messages.hpp>

#include <network.hpp>
#include <rover_can.hpp>
#include "drive_controller.hpp"

DriveController drive_controller;
boost::asio::io_context ctx;

std::chrono::steady_clock::time_point last_heartbeat_sent{};
int heartbeat_interval_ms;

uint16_t subsystem_receive_port;
uint16_t subsystem_update_port;

std::string subsystem_update_ip;
std::chrono::steady_clock::time_point last_message_sent{};
int message_interval_ms;

ControlInformation rover_sensor_information;

void read_subsystem_config(const std::string& fname) {
	namespace ptree = boost::property_tree;
	ptree::ptree subsystem_cfg;
	try {
		ptree::json_parser::read_json(fname, subsystem_cfg);
	} catch (const ptree::json_parser_error& err) {
		std::cerr << "Warning: Using default config after error reading config file: "
				<< err.message() << " (" << err.filename() << ":" << err.line() << ")" << std::endl;
	}
	
	// Reading from empty ptree will apply the default values
	subsystem_receive_port = subsystem_cfg.get<uint16_t>("subsystem.network.recv_port", 22101);

	message_interval_ms = subsystem_cfg.get<int>("subsystem.update_interval_ms", 100);
	subsystem_update_ip = subsystem_cfg.get<std::string>("subsystem.network.update_ip.addr", "239.255.123.123");
	subsystem_update_port = subsystem_cfg.get<uint16_t>("subsystem.network.update_ip.port", 22201);

	heartbeat_interval_ms = subsystem_cfg.get<int>("subsystem.heartbeat_interval_ms", 300);

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
	net::MessageSender sender(ctx);
	try {
		sender.set_destination_endpoint(net::Destination(boost::asio::ip::address::from_string(subsystem_update_ip), subsystem_update_port));
		sender.enable();
	} catch (const boost::system::system_error& err) {
		std::cerr << "Warning: Updates are disabled due to error in IP endpoint: " << err.what() << std::endl;
		sender.disable();
	}

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
	
	// Open can socket
	can_open_socket();

	// Open the CAN Socket
	if (!can_open_socket()) {
		std::cout << "Warning: Unable to open CAN socket\n";
	}

	std::cout << "Initialization complete; Entering main event loop.\n";

	/*
		Main Event Loop
	*/

	try {
		while (operating) {

			ctx.poll();
			drive_controller.update_motor_acceleration();
			
			auto time_now = std::chrono::steady_clock::now();
			std::chrono::duration<double, std::milli> heartbeat_time_passed = time_now - last_heartbeat_sent;
			if (heartbeat_time_passed.count() > heartbeat_interval_ms) {
				can_send(Node::CONTROL_TEENSY, Command::DRIVE_HEARTBEAT_MESSAGE, 0);
				last_heartbeat_sent = time_now;
			}

			can_read_all([] (can_frame* frame) {
				switch (frame->can_id) {
					case can_id(Node::CONTROL_TEENSY, Command::TEENSY_DATA_PACKET_1):
						parse_control_p1(rover_sensor_information, canframe_get_u64(frame));
						break;
					case can_id(Node::CONTROL_TEENSY, Command::TEENSY_DATA_PACKET_2):
						parse_control_p2(rover_sensor_information, canframe_get_u64(frame));
						break;
					case can_id(Node::DRIVE_AXIS_0, Command::GET_VBUS_VOLTAGE):
						unsigned int v = 0;
						for (int i = 0; i < 4; i++) {
							v |= ((unsigned int)frame->data[i] << (8 * i));
						}
						union { unsigned int ul; float fl; } conv = { .ul = v };
						rover_sensor_information.ps_batt = conv.fl;
						break;
				}
			});

			std::chrono::duration<double, std::milli> message_time_passed = time_now - last_message_sent;

			if (message_time_passed.count() >= message_interval_ms) {
				drive_msg::ActualSpeed speed_message;
				speed_message.data.set_left(drive_controller.get_left_speed());
				speed_message.data.set_right(drive_controller.get_right_speed());
				sender.send_message(speed_message);

				drive_msg::DriveMode mode_message;
				::drive::DriveMode_Mode new_mode = static_cast<::drive::DriveMode_Mode>(drive_controller.get_drive_mode());
				mode_message.data.set_mode(new_mode);
				sender.send_message(mode_message);

				sensor_msg::Battery battery_message;
				battery_message.data.set_battery_voltage(rover_sensor_information.ps_batt);
				can_request(Node::DRIVE_AXIS_0, Command::GET_VBUS_VOLTAGE, rover_sensor_information.ps_batt);
				//battery_message.data.set_battery_voltage(can_read_float(Node::DRIVE_AXIS_0, Command::GET_VBUS_VOLTAGE));
				battery_message.data.set_battery_current(rover_sensor_information.main_curr);
				sender.send_message(battery_message);

				sensor_msg::PowerSupply12V ps12_message;
				ps12_message.data.set_v12_supply_voltage(rover_sensor_information.ps12_volt);
				ps12_message.data.set_v12_supply_current(rover_sensor_information.ps12_curr);
				ps12_message.data.set_v12_supply_temperature(rover_sensor_information.temp12);
				sender.send_message(ps12_message);

				sensor_msg::PowerSupply5V ps5_message;
				ps5_message.data.set_v5_supply_voltage(rover_sensor_information.ps5_volt);
				ps5_message.data.set_v5_supply_current(rover_sensor_information.ps5_curr);
				ps5_message.data.set_v5_supply_temperature(rover_sensor_information.temp5);
				sender.send_message(ps5_message);

				sensor_msg::Odrive odrv_message;
				odrv_message.data.set_odrive0_current(rover_sensor_information.odrv0_curr);
				odrv_message.data.set_odrive1_current(rover_sensor_information.odrv1_curr);
				odrv_message.data.set_odrive2_current(rover_sensor_information.odrv2_curr);
				sender.send_message(odrv_message);

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
