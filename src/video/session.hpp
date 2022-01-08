#ifndef SESSION
#define SESSION

//#include "../network/network.hpp"
#include <roversystem/logger.hpp>
#include <roversystem/simpleconfig.hpp>
#include <roversystem/util.hpp>
#include <network.hpp>

#include "camera.hpp"

#include <turbojpeg.h>

const int MAX_STREAMS = 9;
const unsigned int CAMERA_WIDTH = 1280;
const unsigned int CAMERA_HEIGHT = 720;

const int CAMERA_UPDATE_INTERVAL = 5000;

const int CAMERA_FRAME_INTERVAL = 1000 / 15;

const int TICK_INTERVAL = 1000;

const int NETWORK_UPDATE_INTERVAL = 1000 / 2;

struct Config
{
    int base_station_port;
    int rover_port;
    int video_port;

    char base_station_multicast_group[16];
    char rover_multicast_group[16];
    char video_multicast_group[16];
    char interface[16];
};

class Session{
private:
    boost::asio::io_context& ctx;
    net::MessageReceiver control_message_receiver;
public:
    util::Clock global_clock;
    Config config;

    unsigned int frame_counter;
    camera::CaptureSession* streams[MAX_STREAMS] = {0};

    util::Timer camera_update_timer;
    util::Timer tick_timer;
    util::Timer network_update_timer;

    uint32_t ticks;

    tjhandle compressor;
    tjhandle decompressor;

    unsigned int jpeg_quality;
    bool greyscale;
    bool stream_enabled[MAX_STREAMS];

    Session(boost::asio::io_context& ctx);
    ~Session();

    void stderr_handler(logger::Level level, std::string message);
    void load_config(const char* filename);
    int updateCameraStatus();
};

#endif
