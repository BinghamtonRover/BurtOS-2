#include "session.hpp"

boost::asio::io_context net_io_ctx;
VideoConfig session_config;

int main() {
    logger::register_handler(logger::stderr_handler);

    session_config.read("res/v.sconfig");

    Session video_session(session_config, net_io_ctx);
    video_session.update_available_streams();

    // Start with 2 enabled streams
    for (int i = 0; i < 2; i++) {
        video_session.send_stream[i] = true;
    }
    
    for (;;) {
        
        net_io_ctx.poll();

        video_session.send_frames();

        if (video_session.camera_update_timer.ready()) {
            video_session.update_available_streams();
        }

        // Tick.
        video_session.ticks++;
        uint32_t last_tick_interval;
        if (video_session.tick_timer.ready(&last_tick_interval)) {
            video_session.ticks = 0;
        }
    }
}