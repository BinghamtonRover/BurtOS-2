#pragma once

#include <turbojpeg.h>
#include <GL/gl.h>
#include "../../../network/stream.hpp"
#include "../../../video/session.hpp"

class Decoder {
private:
    tjhandle jpeg_decompressor;
public:
    Decoder();
    ~Decoder();
    void decode_frame(net::Frame& frame, unsigned char * out_buffer);
};