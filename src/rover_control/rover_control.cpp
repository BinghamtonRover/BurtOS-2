#include "rover_control.hpp"
#include <network.hpp>
#include <rover_system_messages.hpp>

rc::Drive::Drive(net::MessageSender& ms)
	: sender(ms) {
}

void rc::Drive::set_interval(int milliseconds) {
	interval = milliseconds;
}

int rc::Drive::get_interval() {
	return interval;
}

bool rc::Drive::update_ready() {
	auto time_now = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> time_passed = time_now - last_message_sent;

	return time_passed.count() >= interval;
}

void rc::Drive::send_update() {
	if (actual_drive_mode != requested_drive_mode) {
		set_drive_mode(requested_drive_mode);
	}
	sender.send_message(movement_message);
	last_message_sent = std::chrono::steady_clock::now();
}

void rc::Drive::poll_events() {
	if (update_ready())
		send_update();
}

void rc::Drive::set_drive_mode(::drive::DriveMode_Mode mode) {
	drive_msg::DriveMode mode_message;
	mode_message.data.set_mode(mode);
	sender.send_message(mode_message);

	requested_drive_mode = mode;
}

void rc::Drive::halt() {
	drive_msg::Halt halt_message;
	halt_message.data.set_halt(true);
	sender.send_message(halt_message);
}

void rc::Drive::set_speed(float speed) {
	movement_message.data.set_speed(speed);
}

void rc::Drive::set_angle(float angle) {
	movement_message.data.set_angle(angle);
}

void rc::Drive::set_movement(float speed, float angle) {
	movement_message.data.set_speed(speed);
	movement_message.data.set_angle(angle);
}

void rc::Drive::register_listen_handlers(net::MessageReceiver& m) {
	m.register_handler<drive_msg::ActualSpeed>([this](const uint8_t buf[], std::size_t len) {
		drive::ActualSpeed msg;
		if (msg.ParseFromArray(buf, len)) {
			last_update_received = std::chrono::steady_clock::now();
			if (msg.left() != actual_left_speed || msg.right() != actual_right_speed) {
				actual_left_speed = msg.left();
				actual_right_speed = msg.right();
				EVENT_SPEED(actual_left_speed, actual_right_speed);
			}
		}
	});
	m.register_handler<drive_msg::DriveMode>([this](const uint8_t buf[], std::size_t len) {
		drive::DriveMode msg;
		if (msg.ParseFromArray(buf, len)) {
			last_update_received = std::chrono::steady_clock::now();
			if (msg.mode() != actual_drive_mode) {
				actual_drive_mode = msg.mode();
				EVENT_DRIVEMODE(actual_drive_mode);
			}
		}
	});
}

float rc::Drive::get_actual_left_speed() {
	return actual_left_speed;
}

float rc::Drive::get_actual_right_speed() {
	return actual_right_speed;
}

::drive::DriveMode_Mode rc::Drive::get_actual_drive_mode() {
	return actual_drive_mode;
}



rc::Sensor::Sensor(net::MessageSender& ms)
	: sender(ms) {
}

void rc::Sensor::register_listen_handlers(net::MessageReceiver& m) {
	m.register_handler<sensor_msg::Battery>([this](const uint8_t buf[], std::size_t len) {
		sensor::Battery msg;
		if (msg.ParseFromArray(buf, len)) {
			last_update_received = std::chrono::steady_clock::now();
			if (msg.battery_voltage() != battery_voltage || msg.battery_current() != battery_current) {
				battery_voltage = msg.battery_voltage();
				battery_current = msg.battery_current();
				EVENT_BATTERY_SENSOR(battery_voltage, battery_current);
			}
		}
	});
	m.register_handler<sensor_msg::PowerSupply12V>([this](const uint8_t buf[], std::size_t len) {
		sensor::PowerSupply12V msg;
		if (msg.ParseFromArray(buf, len)) {
			last_update_received = std::chrono::steady_clock::now();
			if (msg.v12_supply_voltage() != v12_supply_voltage || msg.v12_supply_current() != v12_supply_current || msg.v12_supply_temperature() != v12_supply_temperature) {
				v12_supply_voltage = msg.v12_supply_voltage();
				v12_supply_current = msg.v12_supply_current();
				v12_supply_temperature = msg.v12_supply_temperature();
				EVENT_POWERSUPPLY12V_SENSOR(v12_supply_voltage, v12_supply_current, v12_supply_temperature);
			}
		}
	});
	m.register_handler<sensor_msg::PowerSupply5V>([this](const uint8_t buf[], std::size_t len) {
		sensor::PowerSupply5V msg;
		if (msg.ParseFromArray(buf, len)) {
			last_update_received = std::chrono::steady_clock::now();
			if (msg.v5_supply_voltage() != v5_supply_voltage || msg.v5_supply_current() != v5_supply_current || msg.v5_supply_temperature() != v5_supply_temperature) {
				v5_supply_voltage = msg.v5_supply_voltage();
				v5_supply_current = msg.v5_supply_current();
				v5_supply_temperature = msg.v5_supply_temperature();
				EVENT_POWERSUPPLY5V_SENSOR(v5_supply_voltage, v5_supply_current, v5_supply_temperature);
			}
		}
	});
	m.register_handler<sensor_msg::Odrive>([this](const uint8_t buf[], std::size_t len) {
		sensor::Odrive msg;
		if (msg.ParseFromArray(buf, len)) {
			last_update_received = std::chrono::steady_clock::now();
			if (msg.odrive0_current() != odrive0_current || msg.odrive1_current() != odrive1_current || msg.odrive2_current() != odrive2_current) {
				odrive0_current = msg.odrive0_current();
				odrive1_current = msg.odrive1_current();
				odrive2_current = msg.odrive2_current();
				EVENT_ODRIVE_SENSOR(odrive0_current, odrive1_current, odrive2_current);
			}
		}
	});
}

float rc::Sensor::get_battery_voltage() { return battery_voltage; }
float rc::Sensor::get_battery_current() { return battery_current; }
float rc::Sensor::get_v12_supply_voltage() { return v12_supply_voltage; }
float rc::Sensor::get_v12_supply_current() { return v12_supply_current; }
float rc::Sensor::get_v12_supply_temperature() { return v12_supply_temperature; }
float rc::Sensor::get_v5_supply_voltage() { return v5_supply_voltage; }
float rc::Sensor::get_v5_supply_current() { return v5_supply_current; }
float rc::Sensor::get_v5_supply_temperature() { return v5_supply_temperature; }
float rc::Sensor::get_odrive0_current() { return odrive0_current; }
float rc::Sensor::get_odrive1_current() { return odrive1_current; }
float rc::Sensor::get_odrive2_current() { return odrive2_current; }