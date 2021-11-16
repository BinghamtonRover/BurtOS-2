#ifndef CAMERACONTROLMSG_HPP
#define CAMERACONTROLMSG_HPP

//need a newtork import

//Ported from old network
struct CameraControlMessage {
    static const auto TYPE = MessageType::CAMERA_CONTROL;
    enum Setting : uint8_t {
        JPEG_QUALITY,
        GREYSCALE, /* E for consistancy */
        DISPLAY_STATE
    };

    enum sendType : uint8_t {
        DONT_SEND,
        SEND
    };

    Setting setting;

    union {
        uint8_t jpegQuality;
        bool greyscale;
        struct {
            uint8_t stream_index;
            sendType sending;
        } resolution;
    };

    void serialize(Buffer* buffer) {
        network::serialize(buffer, static_cast<uint8_t>(this->setting));
        switch(this->setting){
            case JPEG_QUALITY:
                network::serialize(buffer, this->jpegQuality);
                break;
            case GREYSCALE:
                network::serialize(buffer, this->greyscale);
                break;
            case DISPLAY_STATE:
                network::serialize(buffer, this->resolution.stream_index);
                network::serialize(buffer, static_cast<uint8_t>(
                                            this->resolution.sending));
                break;
        }
        
    }
    void deserialize(Buffer* buffer) {
        network::deserialize(buffer, reinterpret_cast<uint8_t *>(&(this->setting)));
        switch(this->setting){
            case JPEG_QUALITY:
                network::deserialize(buffer, &(this->jpegQuality));
                break;
            case GREYSCALE:
                network::deserialize(buffer, &(this->greyscale));
                break;
            case DISPLAY_STATE:
                network::deserialize(buffer, 
                                    &(this->resolution.stream_index));
                network::deserialize(buffer, reinterpret_cast<uint8_t *>(&(this->resolution.sending)));
                break;
        }
    }
};


#endif