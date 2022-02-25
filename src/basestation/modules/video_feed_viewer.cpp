
#include "video_feed_viewer.hpp"

VideoFeedViewer* vid_feed = nullptr;

VideoFeedViewer::VideoFeedViewer(nanogui::Screen *screen) :
    nanogui::Window(screen, "Rover Feed"),
    decoder()
{
    video_widget = new nanogui::ImageView(this);
    perform_layout(screen->nvg_context());
    vid_feed = this;
}

void VideoFeedViewer::update_frame_STATIC(int stream, net::Frame& frame){
    vid_feed->update_frame(stream, frame);
}

void VideoFeedViewer::update_frame(int stream, net::Frame& frame) {
    uint8_t next_frame_buffer[CAMERA_HEIGHT*CAMERA_WIDTH*3];

    try {
        decoder.decode_frame(frame, next_frame_buffer);
    }
    catch(std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return;
    }

    nanogui::Texture* next_frame = new nanogui::Texture(
            nanogui::Texture::PixelFormat::RGB,
            nanogui::Texture::ComponentFormat::UInt8,
            nanogui::Vector2i(CAMERA_WIDTH, CAMERA_HEIGHT));

    video_widget->set_image(next_frame);
}

