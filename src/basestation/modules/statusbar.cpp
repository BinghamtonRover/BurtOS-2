#include <modules/statusbar.hpp>

#include <nanogui/icons.h>
#include <nanogui/toolbutton.h>
#include <nanogui/screen.h>

#include <basestation.hpp>
#include <modules/console.hpp>
#include <modules/network_settings.hpp>
#include <modules/electrical_info.hpp>

gui::Statusbar::Statusbar(nanogui::Widget* parent) : gui::Toolbar(parent) {
	{
		auto battery_button = new nanogui::ToolButton(right_tray(), FA_BATTERY_FULL);
		battery_button->set_flags(nanogui::Button::NormalButton);
		battery_button->set_callback([this] {
			auto wnd = new gui::ElectricalInfo(screen());
			place_in_right_corner(wnd);
		});
	}
	{
		auto term_button = new nanogui::ToolButton(right_tray(), FA_TERMINAL);
		term_button->set_flags(nanogui::Button::NormalButton);
		term_button->set_callback([this] {
			new Console(screen());
		});
	}
	{
		network_button = new nanogui::ToolButton(right_tray(), FA_NETWORK_WIRED);
		network_button->set_flags(nanogui::Button::NormalButton);
		network_button->set_callback([this] {
			auto wnd = new gui::NetworkSettings(screen());
			place_in_right_corner(wnd);
		});
	}
}

void gui::Statusbar::draw(NVGcontext* ctx) {
	Basestation& bs(Basestation::get());
	
	// Estimate signal strength using the average activity interval
	// Set the symbol color based on strength

	auto last_msg_time = bs.subsystem_feed().latest_activity_time();
	auto now = std::chrono::system_clock::now();

	if (!bs.subsystem_feed().opened()) {
		network_button->set_icon(FA_NETWORK_WIRED);
	} else {
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_msg_time).count() > 800) {
			// Blink when connection is lost
			auto now_steady = std::chrono::steady_clock::now();
			if (now_steady >= next_net_animation) {
				next_net_animation = now_steady + std::chrono::milliseconds(500);

				network_button->set_icon(network_button->icon() == 0 ? FA_WIFI : 0);
			}
		} else {
			network_button->set_icon(FA_WIFI);
		}
	}

	gui::Toolbar::draw(ctx);
}

void gui::Statusbar::place_in_right_corner(nanogui::Window* wnd) {
	auto scr = wnd->screen();
	wnd->set_position(nanogui::Vector2i(scr->width() - wnd->width(), scr->height() - wnd->height() - height()));
}
