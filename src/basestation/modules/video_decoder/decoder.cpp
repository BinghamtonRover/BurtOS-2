
#include "decoder.hpp"

Decoder::Decoder() {
    jpeg_decompressor = tjInitDecompress();
}

Decoder::~Decoder() {
    tjDestroy(jpeg_decompressor);
}

void Decoder::decode_frame( net::Frame& frame, uint8_t* out_buffer) {
    if (!tjDecompress2(
            jpeg_decompressor,
            (unsigned char *)frame.data(),
            (long unsigned int)frame.size(),
            (unsigned char *)out_buffer,
            CAMERA_WIDTH,
            3 * CAMERA_WIDTH,
            CAMERA_HEIGHT,
            TJPF_RGB,
            TJFLAG_NOREALLOC
            )) {
        out_buffer = nullptr;
        throw std::runtime_error("Decoder::frame_decoder: Couldn't decode incoming frame");
    }
}