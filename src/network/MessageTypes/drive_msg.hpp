#ifndef DRIVEMSG_H
#define DRIVEMSG_H

//Prob need a network import

//Ported from old network
struct DriveMessage {
    static const auto TYPE = MessageType::DRIVE;

    int16_t left, right;

    void serialize(Buffer* buffer) {
        network::serialize(buffer, this->left);
        network::serialize(buffer, this->right);
    }

    void deserialize(Buffer* buffer) {
        network::deserialize(buffer, &(this->left));
        network::deserialize(buffer, &(this->right));
    }
};

#endif