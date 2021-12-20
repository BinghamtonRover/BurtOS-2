#pragma once

namespace Network {

enum class MessageType {
	STATUS,
	ARM,
	DRIVE,
	LOG,
	CAMERA,
	CAMERA_CONTROL,
	COUNT //Count is included so we can interate through this enum
};

}