#include "session.hpp"

#include <boost/property_tree/json_parser.hpp>

boost::asio::io_context net_io_ctx;
VideoConfig session_config;

int main() {
    logger::register_handler(logger::stderr_handler);

    namespace tree = boost::property_tree;
    tree::ptree video_cfg;
    tree::json_parser::read_json("cfg/video_config.json", video_cfg);
    if (!session_config.read_from(video_cfg)) {
        return 1;
    }

    Session video_session(session_config, net_io_ctx);
    video_session.update_available_streams();
    
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
