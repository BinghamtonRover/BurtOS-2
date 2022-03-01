#pragma once

#include <nanogui/widget.h>

namespace gui {

// Well, this isn't much more than an alias for a widget
// It comes with a modern theme for placing buttons in the tray
class Tray : public nanogui::Widget {
	public:
		Tray(nanogui::Widget* parent);

};

}
