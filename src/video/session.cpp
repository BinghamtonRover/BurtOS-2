#include "session.hpp"

#include <cstring>
#include <stdexcept>
#include <rover_system_messages.hpp>
#include <filesystem>
#include <iostream>

bool VideoConfig::read_from(boost::property_tree::ptree& src) {
    bool success = true;
    try {

        video_stream_port = src.get<uint16_t>("video.network.stream_ip.port");
        video_stream_address = boost::asio::ip::address_v4::from_string(
            src.get<std::string>("video.network.stream_ip.addr")
        );

        video_command_port = src.get<uint16_t>("video.network.command_ip.port");

        default_jpeg_quality = src.get<uint8_t>("video.camera_init.jpeg_quality", 30);
        default_greyscale_enable = src.get<bool>("video.camera_init.greyscale", false);
        
        std::fill(default_enabled_streams.begin(), default_enabled_streams.end(), false);

        boost::optional enable_streams = src.get_child_optional("video.camera_init.enable_streams");
        if (enable_streams) {
            for (const auto& elem : enable_streams.get()) {
                try {
                    int stream = std::stoi(elem.first);                
                    if (stream >= 0 && stream < MAX_STREAMS) {
                        bool enable = elem.second.get_value<bool>();
                        default_enabled_streams[stream] = enable;
                    } else {
                        throw std::out_of_range("stream index");
                    }
                } catch (const std::logic_error& e) {
                    std::cerr << "Invalid stream index in config: " << elem.first << "\n";
                    success = false;
                }

            }
        }

    } catch (const boost::property_tree::ptree_bad_path& e) {
        std::cerr << "Missing required config value: " << e.what() << "\n";
        success = false;
    } catch (const boost::property_tree::ptree_bad_data& e) {
        std::cerr << "Could not interpret config value: " << e.data<std::string>() << "\n";
        success = false;
    }
    return success;
}

Session::Session(const VideoConfig& config, boost::asio::io_context& ctx) :
    ctrl_message_receiver(config.video_command_port, ctx),
    video_streams_out(ctx),
    cfg(config),
    compressor(tjInitCompress()),
    decompressor(tjInitDecompress()),
    jpeg_quality(cfg.default_jpeg_quality),
    greyscale(cfg.default_greyscale_enable)
{

    if (!compressor || !decompressor) {
        throw std::runtime_error("Session::Session: Could not initialize JPEG compressors");
    }

    util::Clock::init(&global_clock);
    util::Timer::init(&camera_update_timer, CAMERA_UPDATE_INTERVAL, &global_clock);
    util::Timer::init(&tick_timer, TICK_INTERVAL, &global_clock);
    util::Timer::init(&network_update_timer, NETWORK_UPDATE_INTERVAL, &global_clock);

    for (bool& b : send_stream) {
        b = false;
    }

    ctrl_message_receiver.register_handler<video_msg::Quality>([this](const uint8_t buf[], std::size_t len) {
        video::Quality msg;
        if (msg.ParseFromArray(buf, len)) {
            jpeg_quality = msg.jpeg_quality();
            greyscale = msg.grayscale();
        }
    });

    ctrl_message_receiver.register_handler<video_msg::Switch>([this](const uint8_t buf[], std::size_t len) {
        video::Switch msg;
        if (msg.ParseFromArray(buf, len)) {
            if (msg.stream() < MAX_STREAMS) send_stream[msg.stream()] = msg.enabled();
        }
    });
    ctrl_message_receiver.open();

    video_streams_out.create_streams(MAX_STREAMS);
    video_streams_out.set_destination_endpoint(boost::asio::ip::udp::endpoint(
       cfg.video_stream_address,
       cfg.video_stream_port 
    ));

    send_stream = cfg.default_enabled_streams;

}

int Session::update_available_streams() {
    /**
     * We need 2 arrays to keep track of all of our data.
     * 1. An array for new cameras found
     * 2. An array for which camera indices still exist
     **/
    int camerasFound[MAX_STREAMS] = {-1};
    int existingCameras[MAX_STREAMS] = {-1};
    int cntr = 0;
    uint8_t open = 1;
    int numOpen = 0;
    
    // Find all video devices available
    for (int i = 0; cntr < MAX_STREAMS; i++) {
        std::array<char, 32> fname_buffer;
        snprintf(fname_buffer.data(), fname_buffer.size(), "/dev/video%i", i);

        // If video device i doesn't exist, then there are no more
        if (!std::filesystem::exists(fname_buffer.data())) {
            break;
        } else {
            camerasFound[cntr++] = i;
        }
    }
    // There are 3 steps here.
    // 1. Check which cameras exist in the file system.
    // 2. Iterate through our cameras adding any extras that do exist.
    // 3. Remove any cameras that don't exist,
    for (int i = 0; i < cntr; i++) {
        // 1.  Check which cameras exist in the file system.
        for(int j = 0; j < MAX_STREAMS; j++) {
            if(streams[j]) { 
                if(camerasFound[i] == streams[j]->dev_video_id) {
                    // Use -1 to say this camera is being used,
                    // so we don't need to do anything.
                    camerasFound[i] = -1;
                    existingCameras[j] = i;
                    break;
                }
            }
        }
        // Don't initialize this camera, as it exists 
        if (camerasFound[i] == -1) {
            continue;
        }
        numOpen++;

        while(streams[open])
            open++;

        // "/dev/video" is 10 chars long, leave 2 for numbers, and one for null terminator.
        std::array<char, 13> device_name_buffer; 
        snprintf(device_name_buffer.data(), device_name_buffer.size(), "/dev/video%i", camerasFound[i]);

        if (!camera::is_video_device(device_name_buffer.data())) {
            camerasFound[i] = -1;
            continue;
        }
        camera::CaptureSession* cs = new camera::CaptureSession;
        logger::log(logger::DEBUG, "Connecting to camera %d", camerasFound[i]);
        camera::Error err = camera::open(cs, device_name_buffer.data(), CAMERA_WIDTH, CAMERA_HEIGHT, camerasFound[i], &global_clock, CAMERA_FRAME_INTERVAL);
        
        if (err != camera::Error::OK) {
            camerasFound[i] = -1;
            logger::log(logger::DEBUG, "Camera %d errored while opening", cs->dev_video_id);
            delete cs;
            continue;
        }

        // Start the camera.
        err = camera::start(cs);
        if (err != camera::Error::OK) {
            camerasFound[i] = -1;
            logger::log(logger::DEBUG, "Camera %d errored while starting", cs->dev_video_id);
            camera::close(cs);
            delete cs;
            continue;
        }
        // 2. Iterate through our cameras adding any extras that do exist.
        this->streams[open] = cs;
    }
    for(int i = 0; i < cntr; i++) {
        if (camerasFound[i] != -1) {
            logger::log(logger::INFO, "Connected new camera at /dev/video%d", camerasFound[i]);
        }
    }
    // 3. Remove any cameras that don't exist.
    for(int j = 0; j < MAX_STREAMS; j++) {
        if(existingCameras[j] == -1 && this->streams[j] != nullptr) {
            logger::log(logger::INFO, "Camera %d disconnected.", j);
            camera::close(this->streams[j]);
            delete this->streams[j];
            this->streams[j] = nullptr;
        }
    }
    return numOpen;
}

void Session::send_frames() {
    for (size_t i = 1; i < MAX_STREAMS; i++) {
            camera::CaptureSession* cs = streams[i];
            if(!cs || !send_stream[i]) continue;
            
            // Grab a frame.
            uint8_t* frame_buffer;
            size_t frame_size;
            {
                camera::Error err = camera::grab_frame(cs, &frame_buffer, &frame_size);
                if (err != camera::Error::OK) {
                    if (err == camera::Error::AGAIN)
                        continue;

                    logger::log(logger::DEBUG, "Deleting camera %d, because it errored", cs->dev_video_id);
                    camera::close(cs);
                    delete cs;
                    streams[i] = nullptr;
                    continue;
                }
            }
            std::size_t out_frame_size = frame_size;
            // Decode the frame and encode it again to set our desired quality.
            static uint8_t raw_buffer[CAMERA_WIDTH * CAMERA_HEIGHT * 3];
            // Decompress into a raw frame.
            tjDecompress2(
                decompressor,
                frame_buffer,
                frame_size,
                raw_buffer,
                CAMERA_WIDTH,
                3 * CAMERA_WIDTH,
                CAMERA_HEIGHT,
                TJPF_RGB,
                0
            );
            // Recompress into jpeg buffer.
            if (greyscale) {
                tjCompress2(
                    compressor,
                    raw_buffer,
                    CAMERA_WIDTH,
                    3 * CAMERA_WIDTH,
                    CAMERA_HEIGHT,
                    TJPF_RGB,
                    &frame_buffer,
                    &out_frame_size,
                    TJSAMP_GRAY,
                    jpeg_quality,
                    TJFLAG_NOREALLOC
                );
            } else {
                tjCompress2(
                    compressor,
                    raw_buffer,
                    CAMERA_WIDTH,
                    3 * CAMERA_WIDTH,
                    CAMERA_HEIGHT,
                    TJPF_RGB,
                    &frame_buffer,
                    &out_frame_size,
                    TJSAMP_420,
                    jpeg_quality,
                    TJFLAG_NOREALLOC
                );
            }

            video_streams_out.send_frame(i, frame_buffer, out_frame_size);
            
            camera::return_buffer(cs);
        }
}

