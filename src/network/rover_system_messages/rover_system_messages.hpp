#pragma once

#include <messages.hpp>
#include <video_control.pb.h>
#include <drive_control.pb.h>
#include <control_control.pb.h>

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

namespace control_msg {
	DEFINE_MESSAGE_TYPE(Main, control::Main);
	DEFINE_MESSAGE_TYPE(PS12, control::PS12);
	DEFINE_MESSAGE_TYPE(PS5, control::PS5);
	DEFINE_MESSAGE_TYPE(Odrv, control::Odrv);
}

inline void register_messages() {
	msg::register_message_type<video_msg::Quality>();
	msg::register_message_type<video_msg::Switch>();

	msg::register_message_type<drive_msg::Velocity>();
	msg::register_message_type<drive_msg::ActualSpeed>();
	msg::register_message_type<drive_msg::Halt>();
	msg::register_message_type<drive_msg::DriveMode>();

	msg::register_message_type<control_msg::Main>();
	msg::register_message_type<control_msg::PS12>();
	msg::register_message_type<control_msg::PS5>();
	msg::register_message_type<control_msg::Odrv>();
}