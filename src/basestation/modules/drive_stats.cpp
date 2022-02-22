#include <modules/drive_stats.hpp>

#include <sstream>
#include <iomanip>
#include <iostream>

#include <basestation.hpp>
#include <nanogui/nanogui.h>
#include <widgets/layouts/simple_row.hpp>
#include <widgets/layouts/simple_column.hpp>

void gui::DriveStats::perform_layout(NVGcontext* ctx) {
	speed_bar->set_visible(true);
	gui::Window::perform_layout(ctx);

	if (speed_bar->width() < min_bar_size) {
		speed_bar->set_visible(false);
	}

	gui::Window::perform_layout(ctx);
}

void gui::DriveStats::update_speed_displays(float l, float r) {
	float avg = std::abs((l + r) / 2.0F);

	speed_bar->set_value(avg / Basestation::get().remote_drive().throttle());
	
	std::stringstream text_ss;
	text_ss << std::fixed << std::setprecision(2) << avg;
	speed_text->set_value(text_ss.str());
}

gui::DriveStats::DriveStats(nanogui::Screen* screen, nanogui::Vector2i size) : gui::Window(screen, "Drive Stats", true) {
	if (size == 0)
		set_size(200);
	else
		set_size(size);

	auto wnd_layout = new gui::SimpleColumnLayout(margin, margin, 6, gui::SimpleColumnLayout::HorizontalAnchor::STRETCH);
	set_layout(wnd_layout);
	
	auto speeds_layout = new gui::SimpleRowLayout();
	speeds_layout->set_spacing(spacing);

	auto speeds_container = new nanogui::Widget(this);
	speeds_container->set_layout(speeds_layout);

	mode_button = new nanogui::ToolButton(speeds_container, FA_LOCK);
	if (Basestation::get().remote_drive().get_actual_drive_mode() == drive::DriveMode_Mode_DRIVE) {
		mode_button->set_icon(FA_LOCK_OPEN);
		mode_button->set_pushed(false);
	} else {
		mode_button->set_icon(FA_LOCK);
		mode_button->set_pushed(true);
	}

	mode_button->set_change_callback([this](bool pushed) {
		// Shift into Drive when pushed
		if (pushed) {
			Basestation::get().remote_drive().set_drive_mode(drive::DriveMode_Mode_NEUTRAL);
			mode_button->set_icon(FA_LOCK);
			mode_button->set_pushed(true);
		} else {
			Basestation::get().remote_drive().set_drive_mode(drive::DriveMode_Mode::DriveMode_Mode_DRIVE);
			mode_button->set_icon(FA_LOCK_OPEN);
			mode_button->set_pushed(false);
		}
	});

	speed_bar = new nanogui::ProgressBar(speeds_container);
	speed_bar->set_fixed_height(row_height);

	speed_text = new nanogui::TextBox(speeds_container);
	speed_text->set_fixed_height(row_height);
	speed_text->set_fixed_width(80);
	speed_text->set_units("m/s");
	speed_text->set_alignment(nanogui::TextBox::Alignment::Left);
	
	m_parent->perform_layout(screen->nvg_context());

	update_speed_displays(0, 0);
	speeds_update.subscribe(Basestation::get().remote_drive().EVENT_SPEED, [this](float left_speed, float right_speed) {
		update_speed_displays(left_speed, right_speed);
	});

	drive_mode_update.subscribe(Basestation::get().remote_drive().EVENT_DRIVEMODE, [this](drive::DriveMode_Mode new_mode) {
		if (new_mode == drive::DriveMode_Mode_DRIVE) {
			mode_button->set_pushed(false);
			mode_button->set_icon(FA_LOCK_OPEN);
		} else {
			mode_button->set_pushed(true);
			mode_button->set_icon(FA_LOCK);
		}
	});
	
}
