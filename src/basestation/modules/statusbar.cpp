#include <modules/statusbar.hpp>

#include <nanogui/icons.h>
#include <nanogui/toolbutton.h>
#include <nanogui/screen.h>
#include <nanogui/messagedialog.h>

#include <basestation.hpp>
#include <modules/console.hpp>
#include <modules/network_settings.hpp>
#include <modules/electrical_info.hpp>
#include <modules/drive_stats.hpp>
#include <modules/input_config/controller_config.hpp>
#include <modules/video_feed_viewer.hpp>

gui::Statusbar::Statusbar(nanogui::Widget* parent) : gui::Toolbar(parent) {
	{
		auto controller_button = new nanogui::ToolButton(left_tray(), FA_GAMEPAD);
		controller_button->set_flags(nanogui::Button::NormalButton);
		controller_button->set_callback([this] {
			auto wnd = new ControllerConfig(screen(), Basestation::get().controller_manager());
			wnd->center();
		});
		auto video_player_button = new nanogui::ToolButton(left_tray(), FA_VIDEO);
		video_player_button->set_flags(nanogui::Button::NormalButton);
		video_player_button->set_callback([this] {
			auto wnd = new VideoFeedViewer(screen());
			wnd->center();
		});
	}
	{
		auto drive_button = new nanogui::ToolButton(right_tray(), FA_COGS);
		drive_button->set_flags(nanogui::Button::NormalButton);
		drive_button->set_callback([this] {
			auto wnd = new DriveStats(screen());
			wnd->center();
		});
	}
	{
		battery_button = new nanogui::ToolButton(right_tray(), FA_BATTERY_FULL);
		battery_button->set_flags(nanogui::Button::NormalButton);
		battery_button->set_callback([this] {
			auto wnd = new gui::ElectricalInfo(screen());
			place_in_right_corner(wnd);
		});
		battery_event.subscribe(Basestation::get().remote_sensors().EVENT_BATTERY_SENSOR, [this](float v, float c) {
			int icon = FA_BATTERY_EMPTY;
			double charge_percent = (v - 24.5) / (29.4 - 24.5);
			if (charge_percent > 0.9) {
				icon = FA_BATTERY_FULL;
			} else if (charge_percent > 0.70) {
				icon = FA_BATTERY_THREE_QUARTERS;
			} else if (charge_percent > 0.40) {
				icon = FA_BATTERY_HALF;
			} else if (charge_percent > 0.20) {
				icon = FA_BATTERY_QUARTER;
				if (charge_percent > 0.25) {
					low_battery_notified = false;
				}
			} else {
				icon = FA_BATTERY_EMPTY;
				if (!low_battery_notified) {
					low_battery_notified = true;
					new nanogui::MessageDialog(screen(), nanogui::MessageDialog::Type::Warning, "Low Battery", "The battery charge level is below 20%");
				}
			}
			battery_button->set_icon(icon);
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
