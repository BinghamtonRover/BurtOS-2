#include "commands.hpp"

//Can socket number
static int can_socket = 0;
static bool socket_open = false;
static CAN_Status status = CAN_Status::UNSET;

//Send unsigned int data
int can_send(Node device, Command command, unsigned int data) {
    if (is_odrive_device(device)) { return can_send(device, command, 8, get_big_endian(data)); }
    return can_send(device, command, 4, data);
}

//Send int data
int can_send(Node device, Command command, int data) {
    if (is_odrive_device(device)) { return (int)can_send(device, command, 8, get_big_endian((unsigned int)data)); }
    return (int)can_send(device, command, 4, (unsigned int)data);
}

//Send float data
int can_send(Node device, Command command, float data) {
    union { unsigned int ul; float f; } conv = { .f = data };
    if (is_odrive_device(device)) { return can_send(device, command, 8, get_big_endian(conv.ul)); };
    return can_send(device, command, 4, conv.ul);
}

//Send out a can message
int can_send(Node device, Command command, int num_bytes, unsigned int data) {
    //Set status to unset
    status = CAN_Status::UNSET;

#ifdef ONBOARD_CAN_BUS
    //Socket is not open, nothing can send
    if (!socket_open) { return 1; }
    
    //Create the frame, based on specific cases for simplicity
    can_frame frame;
    if (command == Command::SET_CONTROLLER_MODES) {
        frame = get_can_frame(4, device, command, num_bytes, data);
    }
    else {
        frame = get_can_frame(0, device, command, num_bytes, data);
    }

    //Write the frame
    if (write(can_socket, &frame, 16) != 16) {
        status = CAN_Status::FAILED_WRITE;
        return 1;
    }
#endif
    //Successfully sent
    status = CAN_Status::SUCCESS;
    return 0;
}

//Send out a can request read message
int can_request(Node device, Command command, float f) {
    //Set status to unset
    status = CAN_Status::UNSET;

#ifdef ONBOARD_CAN_BUS
    //Socket is not open, nothing can send
    if (!socket_open) { return 1; }
    
    //Create the frame
    can_frame frame;
    frame = get_can_request_frame(0, device, command, f);

    //Write the frame
    if (write(can_socket, &frame, 16) != 16) {
        status = CAN_Status::FAILED_REQUEST;
        return 1;
    }
#endif
    //Successfully sent
    status = CAN_Status::SUCCESS;
    return 0;
}

void can_read_all(const std::function<void(can_frame*)>& callback) {
    if (socket_open) {
        can_frame received_frame;

        while (read(can_socket, &received_frame, sizeof(can_frame)) > 0) {
            callback(&received_frame);
        }
    }
}

uint64_t canframe_get_u64(can_frame* frame) {
    uint64_t x = 0;
    for (int i = 0; i < 8; i++) {
        x |= ((uint64_t)(frame->data[i]) << (8 * (-i + 7)));
    }
    return x;
}

void parse_control_information(ControlInformation& write_to, uint64_t p1, uint64_t p2) {
    parse_control_p1(write_to, p1);
    parse_control_p2(write_to, p2);
}

void parse_control_p1(ControlInformation& write_to, uint64_t p1) {
    //write_to.ps_batt = (float(((0xFF00000000000000l & p1) >> 56) | ((0x00FF000000000000l & p1) >> 40))) / 10.0f;
    write_to.ps12_volt = (float((0x0000FF0000000000l & p1) >> 40)) / 10.0f;
    write_to.ps5_volt = (float((0x000000FF00000000l & p1) >> 32)) / 10.0f;
    write_to.ps12_curr = (float((0x00000000FF000000l & p1) >> 24)) / 10.0f;
    write_to.ps5_curr = (float((0x0000000000FF0000l & p1) >> 16)) / 10.0f;
    write_to.temp12 = (float((0x000000000000FF00l & p1) >> 8)) / 10.0f;
    write_to.temp5 = (float(0x00000000000000FFl & p1)) / 10.0f;   
}

void parse_control_p2(ControlInformation& write_to, uint64_t p2) {
    write_to.odrv0_curr = (float((0xFF00000000000000l & p2) >> 56)) / 10.0f;
    write_to.odrv1_curr = (float((0x00FF000000000000l & p2) >> 48)) / 10.0f;
    write_to.odrv2_curr = (float((0x0000FF0000000000l & p2) >> 40)) / 10.0f;
    write_to.main_curr = (float((0x000000FF00000000l & p2) >> 32)) / 10.0f;
}

void parse_arm_information(ArmInformation& write_to, uint64_t p) {
    write_to.temp1 = (float((0xFF00000000000000l & p) >> 56)) / 10.0f;
    write_to.temp2 = (float((0x00FF000000000000l & p) >> 48)) / 10.0f;
    write_to.temp3 = (float((0x0000FF0000000000l & p) >> 40)) / 10.0f;
    write_to.gyro_x = (float((0x000000FF00000000l & p) >> 32)) / 10.0f;
    write_to.gyro_y = (float((0x00000000FF000000l & p) >> 24)) / 10.0f;
    write_to.gyro_z = (float((0x0000000000FF0000l & p) >> 16)) / 10.0f;
}

void parse_gripper_information(GripperInformation& write_to, uint64_t p) {
    write_to.temp1 = (float((0xFF00000000000000l & p) >> 56)) / 10.0f;
    write_to.temp2 = (float((0x00FF000000000000l & p) >> 48)) / 10.0f;
    write_to.temp3 = (float((0x0000FF0000000000l & p) >> 40)) / 10.0f;
    write_to.gyro_x = (float((0x000000FF00000000l & p) >> 32)) / 10.0f;
    write_to.gyro_y = (float((0x00000000FF000000l & p) >> 24)) / 10.0f;
    write_to.gyro_z = (float((0x0000000000FF0000l & p) >> 16)) / 10.0f;
}

void parse_environmental_analysis_information(EAInformation& write_to, uint64_t p) {
    write_to.temp = (float((0xFF00000000000000l & p) >> 56)) / 10.0f;
    write_to.methane = (float((0x00FF000000000000l & p) >> 48)) / 10.0f;
    write_to.c02 = (float((0x0000FF0000000000l & p) >> 40)) / 10.0f;
    write_to.ph = (float((0x000000FF00000000l & p) >> 32)) / 10.0f;
    write_to.humidity = (float((0x00000000FF000000l & p) >> 24)) / 10.0f;
}

#ifdef ONBOARD_CAN_BUS
//Create a can frame
can_frame get_can_frame(int modifier, Node device, Command command, int num_bytes, unsigned int data) {
    can_frame frame;
    frame.can_id = command | (device << 5) | (modifier << 8);
    frame.can_dlc = num_bytes;
    frame.__pad = 0;
    frame.__res0 = 0;
    frame.__res1 = 0;
    for (int i = 0; i < 4 && i < num_bytes; i++) {
        frame.data[i] = (data >> (8 * (3 - i))) & 0xFF;
    }
    for (int i = 4; i < num_bytes; i++) {
        frame.data[i] = 0;
    }
    return frame;
}

//Create a frame for receiving
can_frame get_can_request_frame(int modifier, Node device, Command command, float f) {
    can_frame frame;
    frame.can_id = (0x40000000) | command | (device << 5) | (modifier << 8);
    frame.can_dlc = 0;
    frame.__pad = 0;
    frame.__res0 = 0;
    frame.__res1 = 0;
    union { unsigned int ul; float fl; } conv = { .fl = f };
    for (int i = 0; i < 4; i++) {
        frame.data[i] = (conv.ul >> (8 * (3 - i))) & 0xFF;
    }
    for (int i = 4; i < 8; i++) {
        frame.data[i] = 0;
    }
    return frame;
}
#endif

//Convert to big endian
unsigned int get_big_endian(unsigned int u) {
	return (((0x000000FF & u) << 24) | ((0x0000FF00 & u) << 8) | ((0x00FF0000 & u) >> 8)  | ((0xFF000000 & u) >> 24));
}

//Open can socket
bool can_open_socket() {
#ifdef ONBOARD_CAN_BUS
    //Get socket number
    can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can_socket < 0) { return false; }

    //Setup ifreq
    struct ifreq ifr;
    strncpy(ifr.ifr_name, "can0", IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
    if (!ifr.ifr_ifindex) { return false; }

    //Don't know what this line does honestly...
    ioctl(can_socket, SIOCGIFINDEX, &ifr);

    //Specifically for opening sockets for CAN
    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = if_nametoindex("can0"); //ifr.ifr_ifindex;

    //Set to "CAN FD mode"
    const int enable_canfd = 1;
    setsockopt(can_socket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd));

    //Set socket to "non-blocking" (for reading)
    int flags = fcntl(can_socket, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(can_socket, F_SETFL, flags);

    //Disable receive filter, then open socket
    if (bind(can_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) { return false; }
#endif
    //Socket has been successfully opened
    socket_open = true;
    return true;
}

//Close can socket
void can_close_socket() {
#ifdef ONBOARD_CAN_BUS
    close(can_socket);
#endif
    socket_open = false;
}

//Get CAN Status
CAN_Status get_can_status() {
    return status;
}

//Check if CAN Status is successful
bool can_status_success() {
    return (status == CAN_Status::SUCCESS);
}

//Check if a device is an odrive
bool is_odrive_device(Node device) {
    return (device >= 1) && (device <= 6);
}
