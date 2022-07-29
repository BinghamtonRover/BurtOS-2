#pragma once

#include <cstring>
#include <chrono>
#include <functional>


#include "constants.hpp"

#ifdef ROVER_CAN_AVAIL
	#include <linux/can.h>
#else
	// Declare that can_frame exists so offline devices can still compile
	extern "C" {
		struct can_frame;
	}
#endif

//Overloads for can_send
int can_send(Node device, Command command, unsigned int data);
int can_send(Node device, Command command, int data);
int can_send(Node device, Command command, float data);

//Send out a CAN message
int can_send(Node device, Command command, int num_bytes, unsigned int data);

//Send out a can request read message
int can_request(Node device, Command command, float f);

//Request to read a CAN message
void can_read_all(const std::function<void(can_frame*)>& read_callback);

constexpr int can_id(Node device, Command command) {
	return command | (device << 5);
}

/// Return the can_id from a can_frame pointer.
///
/// This function allows programs that use rover_can to compile even if 
/// Linux/CAN isn't available. However, programs should try to avoid calling
/// these functions if CAN isn't available since the return value does not
/// make sense.
inline uint32_t can_frame_get_can_id(const can_frame* frame) {
	#ifdef ROVER_CAN_AVAIL
		return frame->can_id;
	#else
		return 0;
	#endif
}

/// Return the data pointer from a can_frame pointer.
///
/// This function allows programs that use rover_can to compile even if 
/// Linux/CAN isn't available. This function returns null if CAN is not
/// available, so users should ensure this function is not called if CAN did not
/// initialize successfully.
inline uint8_t* can_frame_get_data(const can_frame* frame) {
	#ifdef ROVER_CAN_AVAIL
		return frame->data;
	#else
		// Returning null is okay since these functions should never be called if CAN isn't available
		return nullptr;
	#endif
}

uint64_t canframe_get_u64(can_frame* frame);

void parse_control_information(ControlInformation& write_to, uint64_t p1, uint64_t p2);
void parse_control_p1(ControlInformation& write_to, uint64_t p1);
void parse_control_p2(ControlInformation& write_to, uint64_t p2);
void parse_arm_information(ArmInformation& write_to, uint64_t p);
void parse_gripper_information(GripperInformation& write_to, uint64_t p);
void parse_environmental_analysis_information(EAInformation& write_to, uint64_t p);

//Converts unsigned int to big endian
unsigned int get_big_endian(unsigned int u);

//CAN socket
bool can_open_socket();
void can_close_socket();

//CAN status information
CAN_Status get_can_status();
bool can_status_success();

//Check if a device is an odrive
bool is_odrive_device(Node device);
