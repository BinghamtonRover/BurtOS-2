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

















rc::Control::Control(net::MessageSender& ms)
	: sender(ms) {
}

void rc::Control::register_listen_handlers(net::MessageReceiver& m) {
	m.register_handler<control_msg::Main>([this](const uint8_t buf[], std::size_t len) {
		control::Main msg;
		if (msg.ParseFromArray(buf, len)) {
			last_update_received = std::chrono::steady_clock::now();
			if (msg.ps_batt() != ps_batt || msg.main_curr() != main_curr) {
				ps_batt = msg.ps_batt();
				main_curr = msg.main_curr();
				EVENT_MAIN_CONTROL(ps_batt, main_curr);
			}
		}
	});
	m.register_handler<control_msg::PS12>([this](const uint8_t buf[], std::size_t len) {
		control::PS12 msg;
		if (msg.ParseFromArray(buf, len)) {
			last_update_received = std::chrono::steady_clock::now();
			if (msg.ps12_volt() != ps12_volt || msg.ps12_curr() != ps12_curr || msg.temp12() != temp12) {
				ps12_volt = msg.ps12_volt();
				ps12_curr = msg.ps12_curr();
				temp12 = msg.temp12();
				EVENT_PS12_CONTROL(ps12_volt, ps12_curr, temp12);
			}
		}
	});
	m.register_handler<control_msg::PS5>([this](const uint8_t buf[], std::size_t len) {
		control::PS5 msg;
		if (msg.ParseFromArray(buf, len)) {
			last_update_received = std::chrono::steady_clock::now();
			if (msg.ps5_volt() != ps5_volt || msg.ps5_curr() != ps5_curr || msg.temp5() != temp5) {
				ps5_volt = msg.ps5_volt();
				ps5_curr = msg.ps5_curr();
				temp5 = msg.temp5();
				EVENT_PS5_CONTROL(ps5_volt, ps5_curr, temp5);
			}
		}
	});
	m.register_handler<control_msg::Odrv>([this](const uint8_t buf[], std::size_t len) {
		control::Odrv msg;
		if (msg.ParseFromArray(buf, len)) {
			last_update_received = std::chrono::steady_clock::now();
			if (msg.odrv0_curr() != odrv0_curr || msg.odrv1_curr() != odrv1_curr || msg.odrv2_curr() != odrv2_curr) {
				odrv0_curr = msg.odrv0_curr();
				odrv1_curr = msg.odrv1_curr();
				odrv2_curr = msg.odrv2_curr();
				EVENT_ODRV_CONTROL(odrv0_curr, odrv1_curr, odrv2_curr);
			}
		}
	});
}

float rc::Control::get_ps_batt() { return ps_batt; }
float rc::Control::get_main_curr() { return main_curr; }
float rc::Control::get_ps12_volt() { return ps12_volt; }
float rc::Control::get_ps12_curr() { return ps12_curr; }
float rc::Control::get_temp12() { return temp12; }
float rc::Control::get_ps5_volt() { return ps5_volt; }
float rc::Control::get_ps5_curr() { return ps5_curr; }
float rc::Control::get_temp5() { return temp5; }
float rc::Control::get_odrv0_curr() { return odrv0_curr; }
float rc::Control::get_odrv1_curr() { return odrv1_curr; }
float rc::Control::get_odrv2_curr() { return odrv2_curr; }