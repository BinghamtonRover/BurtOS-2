#pragma once

#include <cstring>
#include <chrono>
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
#endif

#include "constants.hpp"

//Max time allowed for reading (in seconds)
constexpr double MAX_READ_TIME = 0.1;

//Overloads for can_send
int can_send(Node device, Command command, unsigned int data);
int can_send(Node device, Command command, int data);
int can_send(Node device, Command command, float data);

//Send out a CAN message
int can_send(Node device, Command command, int num_bytes, unsigned int data);

//Send out a can request read message
int can_request(Node device, Command command);

//Request to read a CAN message
float can_read_float(Node device, Command command);
int can_read_int(Node device, Command command);
long can_read_long(Node device, Command command);

//Recieve a CAN message
unsigned int can_receive(Node device, Command command);
long can_receive_long(Node device, Command command);

//Recieve a CAN frame and check it's ID
bool can_check_hearbeat(Node device);

//Receive all the vital information from each specific teensy
float* get_control_information();
ArmInformation get_arm_information();
GripperInformation get_gripper_information();
EAInformation get_environmental_analysis_information();

#ifdef ONBOARD_CAN_BUS
//Create a can frame
can_frame get_can_frame(int modifier, Node device, Command command, int num_bytes, unsigned int data);
can_frame get_can_request_frame(int modifier, Node device, Command command);
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