#pragma once

// Include protobuf files for convenience
#include "extras.pb.h"

namespace net {

enum class MessageType {
	NONE,
	STATUS,
	ARM,
	DRIVE,
	LOG,
	CAMERA,
	CAMERA_CONTROL,
	STRING_MESSAGE,
	RTT,
	COUNT //Count is included so we can interate through this enum
};

}