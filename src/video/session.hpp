#ifndef SESSION
#define SESSION

#include <roversystem/logger.hpp>
#include <roversystem/simpleconfig.hpp>
#include <roversystem/util.hpp>
#include <network.hpp>
#include <stream.hpp>

#include "camera.hpp"

#include <turbojpeg.h>
#include <array>

const int MAX_STREAMS = 9;
const unsigned int CAMERA_WIDTH = 1280;
const unsigned int CAMERA_HEIGHT = 720;

const int CAMERA_UPDATE_INTERVAL = 5000;

const int CAMERA_FRAME_INTERVAL = 1000 / 15;

const int TICK_INTERVAL = 1000;

const int NETWORK_UPDATE_INTERVAL = 1000 / 2;


struct VideoConfig {
    uint16_t base_station_port;
    uint16_t rover_port;
    uint16_t video_port;
    uint16_t video_command_port;

    char base_station_multicast_group[16];
    char rover_multicast_group[16];
    char video_multicast_group[16];
    char interface[16];

    void read(const char* fname);
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
