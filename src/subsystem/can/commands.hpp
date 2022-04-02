#pragma once

#include <cstring>
#include <chrono>
#include <functional>
#include <fcntl.h>

#ifdef ONBOARD_CAN_BUS
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#else

#warning Compiling rover_can in offline mode

// Declare that can_frame exists so offline devices can still compile
extern "C" {
	struct can_frame;
}

#endif

#include "constants.hpp"

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

uint64_t canframe_get_u64(can_frame* frame);

//Receive all the vital information from each specific teensy
ControlInformation get_control_information();
void parse_control_information(ControlInformation& write_to, uint64_t p1, uint64_t p2);
void parse_control_p1(ControlInformation& write_to, uint64_t p1);
void parse_control_p2(ControlInformation& write_to, uint64_t p2);
void parse_arm_information(ArmInformation& write_to, uint64_t p);
void parse_gripper_information(GripperInformation& write_to, uint64_t p);
void parse_environmental_analysis_information(EAInformation& write_to, uint64_t p);

ArmInformation get_arm_information();
GripperInformation get_gripper_information();
EAInformation get_environmental_analysis_information();

#ifdef ONBOARD_CAN_BUS
//Create a can frame
can_frame get_can_frame(int modifier, Node device, Command command, int num_bytes, unsigned int data);
can_frame get_can_request_frame(int modifier, Node device, Command command, float f);
#endif

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
