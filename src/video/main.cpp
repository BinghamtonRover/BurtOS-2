#include "session.hpp"
#include <stream.hpp>

int main(){
    boost::asio::io_context ctx;

    net::StreamSender video_feeds(ctx);
    video_feeds.create_streams(MAX_STREAMS);

    Session video_session(ctx);
    logger::register_handler(logger::stderr_handler);
    util::Clock::init(&video_session.global_clock);
    video_session.load_config("res/v.sconfig");
    video_session.updateCameraStatus();


    util::Timer::init(&video_session.camera_update_timer, CAMERA_UPDATE_INTERVAL, &video_session.global_clock);
    util::Timer::init(&video_session.tick_timer, TICK_INTERVAL, &video_session.global_clock);
    util::Timer::init(&video_session.network_update_timer, NETWORK_UPDATE_INTERVAL, &video_session.global_clock);
    
    while (true) {
        ctx.poll();
        
        for (size_t i = 1; i < MAX_STREAMS; i++) {
            camera::CaptureSession* cs = video_session.streams[i];
            if(cs == nullptr) continue;
            if(!video_session.stream_enabled[i]) continue;
            // Grab a frame.
            uint8_t* frame_buffer;
            size_t frame_size;
            {
                camera::Error err = camera::grab_frame(cs, &frame_buffer, &frame_size);
                if (err != camera::Error::OK) {
                    if (err == camera::Error::AGAIN)
                        continue;

                    logger::log(logger::DEBUG, "Deleting camera %d, because it errored", video_session.streams[i]->dev_video_id);
                    camera::close(cs);
                    delete cs;
                    video_session.streams[i] = nullptr;
                    continue;
                }
            }
            long unsigned int long_frame_size = frame_size;
            // Decode the frame and encode it again to set our desired quality.
            static uint8_t raw_buffer[CAMERA_WIDTH * CAMERA_HEIGHT * 3];
            // Decompress into a raw frame.
            tjDecompress2(
                video_session.decompressor,
                frame_buffer,
                frame_size,
                raw_buffer,
                CAMERA_WIDTH,
                3 * CAMERA_WIDTH,
                CAMERA_HEIGHT,
                TJPF_RGB,
                0);
            // Recompress into jpeg buffer.
            if (video_session.greyscale) {
                tjCompress2(
                    video_session.compressor,
                    raw_buffer,
                    CAMERA_WIDTH,
                    3 * CAMERA_WIDTH,
                    CAMERA_HEIGHT,
                    TJPF_RGB,
                    &frame_buffer,
                    &long_frame_size,
                    TJSAMP_GRAY,
                    video_session.jpeg_quality,
                    TJFLAG_NOREALLOC);
            } else {
                tjCompress2(
                    video_session.compressor,
                    raw_buffer,
                    CAMERA_WIDTH,
                    3 * CAMERA_WIDTH,
                    CAMERA_HEIGHT,
                    TJPF_RGB,
                    &frame_buffer,
                    &long_frame_size,
                    TJSAMP_420,
                    video_session.jpeg_quality,
                    TJFLAG_NOREALLOC);
            }
            video_feeds.send_frame(i, frame_buffer, frame_size);
            camera::return_buffer(cs);
        }

        if (video_session.camera_update_timer.ready()) {
            video_session.updateCameraStatus();
        }

        // Increment global (across all streams) frame counter. Should be ok. Should...
        video_session.frame_counter++;

    }
}