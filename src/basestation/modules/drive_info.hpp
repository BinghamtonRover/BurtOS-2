#pragma once

#include <nanogui/nanogui.h>
#include <events.hpp>

#include <sstream>
#include <iomanip>
#include <iostream>

namespace gui {

class DriveStats : public nanogui::Window {
	private:
		constexpr static int box_width = 80;
		constexpr static int min_bar_size = 70;
		constexpr static int row_height = 30;
		constexpr static int margin = 6;
		constexpr static int spacing = 4;

		nanogui::ProgressBar* speed_bar;
		nanogui::TextBox* speed_text;
		nanogui::ToolButton* mode_button;

		event::Handler drive_mode_update;
		event::Handler speeds_update;

		void compute_size() {
			int free_width = m_fixed_size.x() - margin * 2 - 2 * spacing - mode_button->width();

			if (free_width - box_width < min_bar_size) {
				speed_bar->set_fixed_size(0);
				speed_bar->set_visible(false);

				speed_text->set_fixed_width(std::min(free_width, box_width));
			} else {
				speed_text->set_fixed_width(box_width);
				speed_bar->set_fixed_width(free_width - box_width);
			}

			m_parent->perform_layout(screen()->nvg_context());
		}

		void update_speed_displays(float l, float r) {
			float avg = (l + r) / 2.0F;

			speed_bar->set_value(avg / Basestation::get().remote_drive().throttle());
			
			std::stringstream text_ss;
			text_ss << std::fixed << std::setprecision(2) << avg;
			speed_text->set_value(text_ss.str());
		}

	public:

		DriveStats(nanogui::Screen* screen, nanogui::Vector2i size = 0) : nanogui::Window(screen, "Drive Stats") {
			if (size == 0)
				set_fixed_width(200);
			else
				set_fixed_size(size);

			auto wnd_layout = new nanogui::BoxLayout(nanogui::Orientation::Vertical);
			wnd_layout->set_margin(margin);
			set_layout(wnd_layout);
			
			auto speeds_layout = new nanogui::GridLayout();
			speeds_layout->set_resolution(3);
			speeds_layout->set_spacing(spacing);

			auto speeds_container = new nanogui::Widget(this);
			speeds_container->set_layout(speeds_layout);

			mode_button = new nanogui::ToolButton(speeds_container, FA_PLAY);
			mode_button->set_flags(nanogui::Button::Flags::NormalButton);
			mode_button->set_change_callback([this](bool pushed) {
				// Shift into Drive when pushed
				if (pushed) {
					Basestation::get().remote_drive().set_drive_mode(drive::DriveMode_Mode::DriveMode_Mode_DRIVE);
				}
			});

			speed_bar = new nanogui::ProgressBar(speeds_container);
			speed_bar->set_fixed_height(row_height);

			speed_text = new nanogui::TextBox(speeds_container);
			speed_text->set_fixed_height(row_height);
			speed_text->set_units("m/s");
			speed_text->set_alignment(nanogui::TextBox::Alignment::Left);
			
			perform_layout(screen->nvg_context());
			compute_size();

			update_speed_displays(0, 0);
			speeds_update.subscribe(Basestation::get().remote_drive().EVENT_SPEED, [this](float left_speed, float right_speed) {
				update_speed_displays(left_speed, right_speed);
			});

			drive_mode_update.subscribe(Basestation::get().remote_drive().EVENT_DRIVEMODE, [this](drive::DriveMode_Mode new_mode) {
				std::cout << "new mode: " << new_mode << "\n";
				mode_button->set_pushed(new_mode == drive::DriveMode_Mode_DRIVE);
			});
			
		}

};

}