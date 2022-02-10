#pragma once

#include <nanogui/nanogui.h>

#include <nanogui/window.h>
#include <nanogui/formhelper.h>

class NetworkConfig : public nanogui::Window {
	public:
		NetworkConfig(nanogui::Screen* screen);
	private:
		nanogui::TextBox* ip_entry;
		nanogui::TextBox* port_entry;
		std::string ip_str;
		int port;
		int interval;
		bool enable;

		int text_entry_width = 150;


};
