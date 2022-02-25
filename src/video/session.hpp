#ifndef SESSION
#define SESSION

#include "roversystem_utils/include/roversystem/logger.hpp"
#include "roversystem_utils/include/roversystem/util.hpp"
#include "../network/network.hpp"
#include "../network/stream.hpp"

#include "camera.hpp"

#include <turbojpeg.h>
#include <boost/property_tree/ptree.hpp>
#include <array>

const int MAX_STREAMS = 9;
const unsigned int CAMERA_WIDTH = 1280;
const unsigned int CAMERA_HEIGHT = 720;

const int CAMERA_UPDATE_INTERVAL = 5000;

const int CAMERA_FRAME_INTERVAL = 1000 / 15;

const int TICK_INTERVAL = 1000;

const int NETWORK_UPDATE_INTERVAL = 1000 / 2;


struct VideoConfig {
    bool read_from(boost::property_tree::ptree& src);

    uint16_t video_stream_port;
    uint16_t video_command_port;
    boost::asio::ip::address_v4 video_stream_address;

    uint8_t default_jpeg_quality;
    bool default_greyscale_enable;
    std::array<bool, MAX_STREAMS> default_enabled_streams;
};

class Session {
private:
    net::MessageReceiver ctrl_message_receiver;
    net::StreamSender video_streams_out;
    const VideoConfig& cfg;
public:
    util::Clock global_clock;
    VideoConfig config;

    camera::CaptureSession* streams[MAX_STREAMS] = {0};

    util::Timer camera_update_timer;
    util::Timer tick_timer;
    util::Timer network_update_timer;

    uint32_t ticks = 0;

    tjhandle compressor;
    tjhandle decompressor;

    unsigned int jpeg_quality = 30;
    bool greyscale = false;
    std::array<bool, MAX_STREAMS> send_stream;

    Session(const VideoConfig&, boost::asio::io_context&);

    int update_available_streams();
    void send_frames();
};

#endif
