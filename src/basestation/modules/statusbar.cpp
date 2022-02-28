#include <modules/statusbar.hpp>

#include <nanogui/icons.h>
#include <nanogui/toolbutton.h>
#include <nanogui/screen.h>

#include <modules/console.hpp>
#include <modules/network_settings.hpp>

gui::Statusbar::Statusbar(nanogui::Widget* parent) : gui::Toolbar(parent) {
	{
		auto term_button = new nanogui::ToolButton(right_tray(), FA_TERMINAL);
		term_button->set_flags(nanogui::Button::NormalButton);
		term_button->set_callback([this] {
			new Console(screen());
		});
	}
	{
		auto network_button = new nanogui::ToolButton(right_tray(), FA_NETWORK_WIRED);
		network_button->set_flags(nanogui::Button::NormalButton);
		network_button->set_callback([this] {
			auto wnd = new gui::NetworkSettings(screen());
			place_in_right_corner(wnd);
		});
	}
}

void gui::Statusbar::place_in_right_corner(nanogui::Window* wnd) {
	auto scr = wnd->screen();
	wnd->set_position(nanogui::Vector2i(scr->width() - wnd->width(), scr->height() - wnd->height() - height()));
}
