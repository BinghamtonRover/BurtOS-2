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
int can_request(Node device, Command command) {
    //Set status to unset
    status = CAN_Status::UNSET;

#ifdef ONBOARD_CAN_BUS
    //Socket is not open, nothing can send
    if (!socket_open) { return 1; }
    
    //Create the frame
    can_frame frame;
    frame = get_can_request_frame(0, device, command);

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

//Request to read a CAN message, with a float output
float can_read_float(Node device, Command command) {
    //Read value
    unsigned int received = 0;
    received = can_receive(device, command);

    //Convert convert from big endian (conversion is symmetrical) if not from an odrive
    if (!is_odrive_device(device)) { received = get_big_endian(received); }
    
    //Convert unsigned int back to float, then return
    union { unsigned int ul; float f; } conv = { .ul = received };
    return conv.f;
}

//Request to read a CAN message, with an int output
int can_read_int(Node device, Command command) {
    //Read value
    unsigned int received = 0;
    received = can_receive(device, command);

    //Convert convert from big endian (conversion is symmetrical) if not from an odrive, and return
    if (!is_odrive_device(device)) { received = get_big_endian(received); }
    return (int)received;
}

//Request to read a CAN message, with an long output
long can_read_long(Node device, Command command) {
    //Do not allow odrive to read longs (this can be changed in the future if needed)
    if (is_odrive_device(device)) { 
        status = CAN_Status::FAILED_READ;
        return 0ul;
    }

    //Read value
    long received = 0;
    received = can_receive_long(device, command);
    return received;
}

//receive a CAN message
unsigned int can_receive(Node device, Command command) {
    //Define return value
    unsigned int return_value = 0;

    //Set status to unset
    status = CAN_Status::UNSET;

#ifdef ONBOARD_CAN_BUS
    //Socket is not open, nothing can read
    if (!socket_open) { return 1; }
    
    //Create the frame
    can_frame frame;
    frame = get_can_frame(0, device, command, 0, 0);
    int expected_can_id = command | (device << 5);
    frame.can_id = expected_can_id;

    //Request the frame (odrive only)
    if (is_odrive_device(device)) {
        can_request(device, command);
        if (!can_status_success()) {
            return 0;
        }
    }

    //Read the frame
    int read_size = 0;
    frame.can_id = 0;
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
	std::chrono::duration<double> delta_time = start_time - start_time;
    
    while ((read_size <= 0 || frame.can_id != expected_can_id) && delta_time.count() <= MAX_READ_TIME) {
        read_size = read(can_socket, &frame, sizeof(struct can_frame));
        delta_time = std::chrono::steady_clock::now() - start_time;
    }

    //Check if invalid read
    if (read_size <= 0 || frame.can_id != expected_can_id) {
        status = CAN_Status::FAILED_READ;
        return 0;
    }

    //Put data into unsigned integer
    for (int i = 0; i < 4; i++) {
        return_value |= ((unsigned int)(frame.data[i]) << (8 * i));
    }
    
#endif
    //Successfully read
    status = CAN_Status::SUCCESS;
    return return_value;
}

//receive a CAN message
long can_receive_long(Node device, Command command) {
    //Define return value
    unsigned int return_value = 0;

    //Set status to unset
    status = CAN_Status::UNSET;

#ifdef ONBOARD_CAN_BUS
    //Socket is not open, nothing can read
    if (!socket_open) { return 1; }
    
    //Create the frame
    can_frame frame;
    frame = get_can_frame(0, device, command, 0, 0);
    int expected_can_id = command | (device << 5);
    frame.can_id = expected_can_id;

    //Set the filter
    struct can_filter rfilter[1];
	rfilter[0].can_id = expected_can_id;
	rfilter[0].can_mask = expected_can_id;
	setsockopt(can_socket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

    //Read the frame
    int read_size = 0;
    while (read_size <= 0 && frame.can_id != expected_can_id) {
        read_size = read(can_socket, &frame, sizeof(struct can_frame));
    }

    //Check if invalid read
    if (read_size <= 0) {
        status = CAN_Status::FAILED_READ;
        return 0;
    }

    //Put data into long
    for (int i = 0; i < read_size; i++) {
        return_value |= ((long)(frame.data[i]) << (8 * i));
    }
    
#endif
    //Successfully read
    status = CAN_Status::SUCCESS;
    return return_value;
}

//Recieve a CAN frame and check it's ID (for now, this only works for odrives)
bool can_check_hearbeat(Node device) {
    //Set status to unset
    status = CAN_Status::UNSET;
    bool return_value = true;

#ifdef ONBOARD_CAN_BUS
    //Socket is not open, nothing can read
    if (!socket_open) { return 1; }
    
    //Create a frame
    can_frame frame;

    //Read current frame
    int expected_can_id = (Command::DRIVE_HEARTBEAT_MESSAGE) || (device << 5);
    int read_size = read(can_socket, &frame, sizeof(struct can_frame));

    //Check if invalid read
    if (read_size < 0) {
        status = CAN_Status::FAILED_READ;
        return 0;
    }
    
    //Set return value
    return_value = (frame.can_id == expected_can_id);

#endif
    //Successfully read
    status = CAN_Status::SUCCESS;
    return return_value;
}

//Receive all the vital information from the control teensy
ControlInformation get_control_information() {
    //return information
    ControlInformation ret;

    //fetch information
    long p1 = can_read_long(Node::CONTROL_TEENSY, Command::TEENSY_DATA_PACKET_1);
    if (!can_status_success()) { return ret; }
    long p2 = can_read_long(Node::CONTROL_TEENSY, Command::TEENSY_DATA_PACKET_2);
    if (!can_status_success()) { return ret; }

    //after successful reads, interpret the data
    ret.ps_batt = (float(((0xFF00000000000000l & p1) >> 56) | ((0x00FF000000000000l & p1) >> 40))) / 10.0f;
    ret.ps12_volt = (float((0x0000FF0000000000l & p1) >> 40)) / 10.0f;
    ret.ps5_volt = (float((0x000000FF00000000l & p1) >> 32)) / 10.0f;
    ret.ps12_curr = (float((0x00000000FF000000l & p1) >> 24)) / 10.0f;
    ret.ps5_curr = (float((0x0000000000FF0000l & p1) >> 16)) / 10.0f;
    ret.temp12 = (float((0x000000000000FF00l & p1) >> 8)) / 10.0f;
    ret.temp5 = (float(0x00000000000000FFl & p1)) / 10.0f;
    ret.odrv0_curr = (float((0xFF00000000000000l & p2) >> 56)) / 10.0f;
    ret.odrv1_curr = (float((0x00FF000000000000l & p2) >> 48)) / 10.0f;
    ret.odrv2_curr = (float((0x0000FF0000000000l & p2) >> 40)) / 10.0f;
    ret.main_curr = (float((0x000000FF00000000l & p2) >> 32)) / 10.0f;
    return ret;
}

//Receive all the information from the arm
ArmInformation get_arm_information() {
    //return information
    ArmInformation ret;

    //fetch information
    long p = can_read_long(Node::ARM_TEENSY, Command::TEENSY_DATA_PACKET_1);
    if (!can_status_success()) { return ret; }

    //after a successful read, interpret the data
    ret.temp1 = (float((0xFF00000000000000l & p) >> 56)) / 10.0f;
    ret.temp2 = (float((0x00FF000000000000l & p) >> 48)) / 10.0f;
    ret.temp3 = (float((0x0000FF0000000000l & p) >> 40)) / 10.0f;
    ret.gyro_x = (float((0x000000FF00000000l & p) >> 32)) / 10.0f;
    ret.gyro_y = (float((0x00000000FF000000l & p) >> 24)) / 10.0f;
    ret.gyro_z = (float((0x0000000000FF0000l & p) >> 16)) / 10.0f;
    return ret;
}

//Receive all the information from the gripper
GripperInformation get_gripper_information() {
    //return information
    GripperInformation ret;

    //fetch information
    long p = can_read_long(Node::GRIPPER_TEENSY, Command::TEENSY_DATA_PACKET_1);
    if (!can_status_success()) { return ret; }

    //after a successful read, interpret the data
    ret.temp1 = (float((0xFF00000000000000l & p) >> 56)) / 10.0f;
    ret.temp2 = (float((0x00FF000000000000l & p) >> 48)) / 10.0f;
    ret.temp3 = (float((0x0000FF0000000000l & p) >> 40)) / 10.0f;
    ret.gyro_x = (float((0x000000FF00000000l & p) >> 32)) / 10.0f;
    ret.gyro_y = (float((0x00000000FF000000l & p) >> 24)) / 10.0f;
    ret.gyro_z = (float((0x0000000000FF0000l & p) >> 16)) / 10.0f;
    return ret;
}

//Receive all the information for environmental analysis
EAInformation get_environmental_analysis_information() {
    //return information
    EAInformation ret;

    //fetch information
    long p = can_read_long(Node::EA_TEENSY, Command::TEENSY_DATA_PACKET_1);
    if (!can_status_success()) { return ret; }

    //after a successful read, interpret the data
    ret.temp = (float((0xFF00000000000000l & p) >> 56)) / 10.0f;
    ret.methane = (float((0x00FF000000000000l & p) >> 48)) / 10.0f;
    ret.c02 = (float((0x0000FF0000000000l & p) >> 40)) / 10.0f;
    ret.ph = (float((0x000000FF00000000l & p) >> 32)) / 10.0f;
    ret.humidity = (float((0x00000000FF000000l & p) >> 24)) / 10.0f;
    return ret;
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
can_frame get_can_request_frame(int modifier, Node device, Command command) {
    can_frame frame;
    frame.can_id = (0x40000000) | command | (device << 5) | (modifier << 8);
    frame.can_dlc = 0;
    frame.__pad = 0;
    frame.__res0 = 0;
    frame.__res1 = 0;
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
    addr.can_ifindex = ifr.ifr_ifindex;

    //Set to "CAN FD mode"
    // TODO: Test if this is still needed (probably not?)
    //int enable_canfd = 1;
    //setsockopt(can_socket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd));

    //Set socket to "non-blocking" (for reading)
    int flags = fcntl(can_socket, F_GETFL);
    flags |= ~(O_NONBLOCK);
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
