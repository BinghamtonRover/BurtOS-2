#include "rover_control.hpp"
#include <network.hpp>
#include <rover_system_messages.hpp>

Drive::Drive(net::MessageSender& ms) {
    sender = ms;
}

void Drive::set_interval(int milliseconds) {
	interval = milliseconds;
}

void Drive::get_interval() {
    return interval;
}

void Drive::poll_events() {
    auto time_now = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> time_passed = time_now - last_message_sent;

    if (time_passed.count() >= interval) {
        sender.send_message(movement_message);
    }
}

void set_drive_mode(drive_msg::DriveMode mode) {
    drive_msg::DriveMode mode_message;
    mode_message.data.set_drive_mode(mode);
    sender.send_message(mode_message);
}

void halt() {
    drive_msg::Halt halt_message;
    halt_message.data.set_halt(true);
    sender.send_message(halt_message);
}

void set_speed(float speed) {
    drive_msg::Velocity speed_message;
    speed_message.data.set_speed(speed);
    sender.send_message(speed_message);
}

void set_angle(float angle) {
    drive_msg::Velocity angle_message;
    angle_message.data.set_angle(angle);
    sender.send_message(angle_message);
}

void set_movement(float speed, float angle) {
    movement_message.set_speed(speed);
    movement_message.set_angle(angle);
}
