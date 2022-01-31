#pragma once

#include <cstring>

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

//Methods for can_send
int can_send(Node device, Command command, float data);
int can_send(Node device, Command command, int data);

//Send out a can message
int can_send(Node device, Command command, int num_bytes, unsigned long data);

//Recieve a message using can_send
int can_receive(Node device, Command command);

#ifdef ONBOARD_CAN_BUS
//Create a can frame
canfd_frame get_can_frame(int modifier, Node device, Command command, int num_bytes, unsigned long data);
#endif

//Take the first 4 bytes of an unsigned long, and convert them to big endian
unsigned long get_big_endian(unsigned long u);

//Can socket
bool can_open_socket();
void can_close_socket();