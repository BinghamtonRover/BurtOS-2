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
