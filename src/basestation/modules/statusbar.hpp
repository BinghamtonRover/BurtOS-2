/*
	Collection of classes used for the status toolbar utility, including
	1. The toolbar itself
	2. Working on it...
*/

#pragma once

#include <widgets/toolbar.hpp>
#include <events.hpp>
#include <chrono>

namespace gui {

/*
	The main toolbar for base station status shown across the bottom of every screen
*/
class Statusbar : public gui::Toolbar {
	public:
		Statusbar(nanogui::Widget* parent);

		virtual void draw(NVGcontext* ctx) override;

	private:
		void place_in_right_corner(nanogui::Window* wnd);

		nanogui::Button* network_button;
		nanogui::Button* battery_button;
		std::chrono::steady_clock::time_point next_net_animation;

		event::Handler battery_event;
		// Start with true so the initial value won't send a notification
		bool low_battery_notified = true;
};

}