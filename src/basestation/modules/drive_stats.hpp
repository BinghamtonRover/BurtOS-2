#pragma once

#include <events.hpp>

#include <widgets/window.hpp>
#include <nanogui/vector.h>
#include <nanogui/common.h>

namespace gui {

class DriveStats : public gui::Window {
	public:
		DriveStats(nanogui::Screen* screen, nanogui::Vector2i size = 0);

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

		void compute_size();

		void update_speed_displays(float l, float r);

};

}