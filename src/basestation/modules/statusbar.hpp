/*
	Collection of classes used for the status toolbar utility, including
	1. The toolbar itself
	2. Working on it...
*/

#pragma once

#include <widgets/toolbar.hpp>

namespace gui {

/*
	The main toolbar for base station status shown across the bottom of every screen
*/
class Statusbar : public gui::Toolbar {
	public:
		Statusbar(nanogui::Widget* parent);
	private:
		void place_in_right_corner(nanogui::Window* wnd);
};

}