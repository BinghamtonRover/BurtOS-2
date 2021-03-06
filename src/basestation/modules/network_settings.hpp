#pragma once

#include <string>
#include <stdexcept>

#include <widgets/window.hpp>

namespace gui {

class NetworkSettings : public gui::Window {
	public:
		NetworkSettings(nanogui::Screen* screen);
	private:
		// Most of these variables are for the form helper widgets but aren't really needed
		std::string ip_str;
		std::string mcast_feed_str;
		int mcast_feed_port;
		int port;
		int interval;
		bool enable;
		bool subsys_feed_enable;
		bool subsys_feed_mcast;

		int text_entry_width = 150;

		void open_error_popup(const std::exception& err);


};

}	// namespace gui
