#include "commands.hpp"

//Can socket number
static int can_socket = 0;
static bool socket_open = false;

//Send float data
int can_send(Node device, Command command, float data) {
    union { unsigned long ul; float f; } conv = { .f = data };
    return can_send(device, command, 8, get_big_endian_first_half(conv.ul));
}

//Send unsigned in data
int can_send(Node device, Command command, int data) {
    return can_send(device, command, 4, get_big_endian_first_half((unsigned long)data));
}

//Send out a can message
int can_send(Node device, Command command, int num_bytes, unsigned long data) {
    //Socket is not open, nothing can send
    if (!socket_open) { return 1; }
    
    //Create the frame, based on specific cases for simplicity
    canfd_frame frame;
    switch (command) {
        case Command::SET_AXIS_REQUESTED_STATE:
            frame = get_can_frame(0, device, command, num_bytes, data);
            break;
        case Command::SET_INPUT_VEL:
            frame = get_can_frame(0, device, command, num_bytes, data);
            break;
        case Command::SET_CONTROLLER_MODES:
            frame = get_can_frame(4, device, command, num_bytes, data);
            break;
        default:
            return 1;
    }

    //Write the frame
    if (write(can_socket, &frame, 10) != 10) {
        //Failed to write
        return 1;
    }

    //Successfully sent
    return 0;
}

//Create a can frame
canfd_frame get_can_frame(int modifier, Node device, Command command, int num_bytes, unsigned long data) {
    canfd_frame frame;
    frame.can_id = command | (device << 5) | (modifier << 8);
    frame.len = num_bytes;
    frame.flags = 0;
    frame.__res0 = 0;
    frame.__res1 = 0;
    for (int i = 0; i < 8; i++) {
        frame.data[i] = (data >> (8 * (7 - i))) & 0xFF;
    }
    return frame;
}


//Take the first 4 bytes of an unsigned long, and convert them to big endian
unsigned long get_big_endian_first_half(unsigned long u) {
	return (((0x000000FF & u) << 24) | ((0x0000FF00 & u) << 8) | ((0x00FF0000 & u) >> 8)  | ((0xFF000000 & u) >> 24)) << 32;
}

//Open can socket
bool open_can_socket() {
    //Get socket number
    can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can_socket < 0) { return false; }

    //Setup ifreq
    struct ifreq ifr;
    strncpy(ifr.ifr_name, "can0", IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
    if (!ifr.ifr_ifindex) { return false; }

    //Specifically for opening sockets for CAN
    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    //Disable recieve filter, then open socket
    setsockopt(can_socket, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
    if (bind(can_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) { return false; }

    //Socket has been successfully opened
    socket_open = true;
    return true;
}

//Close can socket
void close_can_socket() {
    close(can_socket);
    socket_open = false;
}