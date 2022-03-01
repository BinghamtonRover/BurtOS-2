#pragma once
//https://docs.google.com/document/d/19qzwL-YT_U8nMIzBIrLZtw7b7TNUX8SrNCQFAcU3jeM/edit
//Specific information on nodes and commands here

enum Node {
    CONTROL_TEENSY = 0x0,
    DRIVE_AXIS_0 = 0x1,
    DRIVE_AXIS_1 = 0x2,
    DRIVE_AXIS_2 = 0x3,
    DRIVE_AXIS_3 = 0x4,
    DRIVE_AXIS_4 = 0x5,
    DRIVE_AXIS_5 = 0x6,
    RASPBERRY_PI = 0x7,
    ARM_TEENSY = 0x8,
    GRIPPER_TEENSY = 0x9,
    EA_TEENSY = 0xA
};

enum Command {
    CAN_OPEN_NMT_MESSAGE = 0x0,
    DRIVE_HEARTBEAT_MESSAGE = 0x1,
    DRIVE_EMERGENCY_STOP_MESSAGE = 0x2,
    GET_MOTOR_ERROR = 0x3,
    GET_ENCODER_ERROR = 0x4,
    GET_SENSORLESS_ERROR = 0x5,
    SET_AXIS_NODE_ID = 0x6,
    SET_AXIS_REQUESTED_STATE = 0x7,
    SET_AXIS_STARTUP_CONFIG = 0x8,
    GET_ENCODER_ESTIMATES = 0x9,
    GET_ENCODER_COUNT = 0xA,
    SET_CONTROLLER_MODES = 0xB,
    SET_INPUT_POS = 0xC,
    SET_INPUT_VEL = 0xD,
    SET_INPUT_TORQUE = 0xE,
    SET_LIMITS = 0xF,
    START_ANTICOGGING = 0x10,
    SET_TRAJ_VEL_LIMIT = 0x11,
    SET_TRAJ_ACCEL_LIMIT = 0x12,
    SET_TRAJ_INERTIA = 0x13,
    GET_IQ = 0x14,
    GET_SENSORLESS_ESTIMATES = 0x15,
    REBOOT_DRIVE = 0x16,
    GET_VBUS_VOLTAGE = 0x17,
    CLEAR_ERRORS = 0x18,
    SET_LINEAR_COUNT = 0x19,
    SET_POSITION_GAIN = 0x1A,
    SET_VEL_GAINS = 0x1B,
    TEENSY_DATA_PACKET_1 = 0x1C,
    TEENSY_DATA_PACKET_2 = 0x1D
};

enum CAN_Status {
    UNSET = 0x0,
    SUCCESS = 0x1,
    FAILED_READ = 0x2,
    FAILED_WRITE = 0x3,
    FAILED_REQUEST = 0x4
};

struct ControlInformation {
    float ps_batt;
    float ps12_volt;
    float ps5_volt;
    float ps12_curr;
    float ps5_curr;
    float temp12;
    float temp5;
    float odrv0_curr;
    float odrv1_curr;
    float odrv2_curr;
    float main_curr;
};

struct ArmInformation {
    float temp1;
    float temp2;
    float temp3;
    float gyro_x;
    float gyro_y;
    float gyro_z;
};

struct GripperInformation {
    float temp1;
    float temp2;
    float temp3;
    float gyro_x;
    float gyro_y;
    float gyro_z;
};

struct EAInformation {
    float temp;
    float methane;
    float c02;
    float ph;
    float humidity;
};
