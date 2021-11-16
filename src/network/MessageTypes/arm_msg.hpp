#ifndef ARMMSG_H
#ifndef ARMMSG_H

//Prob needs a network import

//Arm Message just ported from old system
struct ArmMessage {
    static const auto TYPE = MessageType::ARM;

    enum class Joint : uint8_t {
        BASE_ROTATE,
        BASE_SHOULDER,
        ELBOW, 
        WRIST,
        GRIPPER_ROTATE,
        GRIPPER_FINGERS
    };

    enum class Movement : uint8_t {
        STOP,
        CLOCK,
        COUNTER
    };

    int16_t joint, movement;

    void serialize(Buffer* buffer) {
        network::serialize(buffer, this->joint);
        network::serialize(buffer, this->movement);
    }

    void deserialize(Buffer* buffer) {
        network::deserialize(buffer, &(this->joint));
        network::deserialize(buffer, &(this->movement));
    }
};

#endif