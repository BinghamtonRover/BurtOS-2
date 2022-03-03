#pragma once

#include <messages.hpp>
#include <video_control.pb.h>
#include <drive_control.pb.h>
#include <sensor_control.pb.h>

namespace video_msg {
	DEFINE_MESSAGE_TYPE(Quality, video::Quality)
	DEFINE_MESSAGE_TYPE(Switch, video::Switch)
}

namespace drive_msg {
	DEFINE_MESSAGE_TYPE(Velocity, drive::Velocity)
	DEFINE_MESSAGE_TYPE(ActualSpeed, drive::ActualSpeed)
	DEFINE_MESSAGE_TYPE(Halt, drive::Halt)
	DEFINE_MESSAGE_TYPE(DriveMode, drive::DriveMode)
}

namespace sensor_msg {
	DEFINE_MESSAGE_TYPE(Battery, sensor::Battery)
	DEFINE_MESSAGE_TYPE(PowerSupply12V, sensor::PowerSupply12V)
	DEFINE_MESSAGE_TYPE(PowerSupply5V, sensor::PowerSupply5V)
	DEFINE_MESSAGE_TYPE(Odrive, sensor::Odrive)
}

inline void register_messages() {
	msg::register_message_type<video_msg::Quality>();
	msg::register_message_type<video_msg::Switch>();

	msg::register_message_type<drive_msg::Velocity>();
	msg::register_message_type<drive_msg::ActualSpeed>();
	msg::register_message_type<drive_msg::Halt>();
	msg::register_message_type<drive_msg::DriveMode>();

	msg::register_message_type<sensor_msg::Battery>();
	msg::register_message_type<sensor_msg::PowerSupply12V>();
	msg::register_message_type<sensor_msg::PowerSupply5V>();
	msg::register_message_type<sensor_msg::Odrive>();
}