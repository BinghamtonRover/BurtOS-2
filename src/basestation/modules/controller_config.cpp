#include "controller_config.hpp"

float ControllerConfig::to_percent_scale(float axis_value) {
	return (axis_value - JoystickAxis::AXIS_MIN) / JoystickAxis::AXIS_RANGE;
}

float ControllerConfig::to_axis_scale(float percent) {
	return percent * JoystickAxis::AXIS_RANGE + JoystickAxis::AXIS_MIN;
}

ControllerConfig::ControllerConfig(nanogui::Screen* screen, ControllerManager& mgr) :
		nanogui::Window(screen, "Controllers"),
		mgr(mgr) {

	set_layout(new nanogui::GroupLayout());
	set_fixed_width(800);

	devices_selector = new nanogui::ComboBox(this);
	devices_selector->set_icon(FA_GAMEPAD);
	devices_selector->set_caption("  (no controllers detected)");
	devices_selector->set_callback([this](int) {
		recreate_axes_table();
	});

	table_scroller = new nanogui::VScrollPanel(this);
	table_scroller->set_fixed_height(400);
	axes_table = new nanogui::Widget(table_scroller);
	axes_table->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 4, nanogui::Alignment::Middle, 0, 6));

	auto option_bar = new nanogui::Widget(this);
	option_bar->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 16));

	auto close_button = new nanogui::Button(option_bar, "Close");
	close_button->set_icon(FA_WINDOW_CLOSE);
	close_button->set_callback([this]() {
		this->screen()->dispose_window(this);
	});

	refresh();

	this->screen()->perform_layout();
	
}

void ControllerConfig::recreate_axes_table() {
	axis_table_entries.clear();
	while (axes_table->child_count()) {
		axes_table->remove_child_at(0);
	}
	const char* col_title_font = "sans-bold";
	int col_title_size = 24;
	new nanogui::Label(axes_table, "Axis", col_title_font, col_title_size);
	new nanogui::Label(axes_table, "Binding", col_title_font, col_title_size);
	new nanogui::Label(axes_table, "Calibrate", col_title_font, col_title_size);
	new nanogui::Label(axes_table, "Value", col_title_font, col_title_size);

	if (devices_selector->enabled()) {
		Controller& selected_dev = mgr.devices()[device_js_ids[devices_selector->selected_index()]];
		int axis_num = 0;
		for (auto& axis : selected_dev.axes()) {
			new nanogui::Label(axes_table, std::to_string(axis_num));
			auto bind_name = new nanogui::Label(axes_table, axis.action().name);

			auto calibrate_menu = new nanogui::Widget(axes_table);
			calibrate_menu->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 6));

			// These buttons will copy the axis pointer
			auto axis_ptr = &axis;

			auto b_toggle_calibrating = new nanogui::ToolButton(calibrate_menu, FA_PLAY);
			b_toggle_calibrating->set_callback([axis_ptr, b_toggle_calibrating]() {
				// The icon will be wrong if calibration is modified with console, but the action will
				// be correct
				if (axis_ptr->action().name == "Calibrating") {
					b_toggle_calibrating->set_icon(FA_PLAY);
					axis_ptr->end_calibration();
				} else {
					b_toggle_calibrating->set_icon(FA_STOP);
					axis_ptr->start_calibration();
				}
				b_toggle_calibrating->set_pushed(false);
			});

			auto b_reset_calibration = new nanogui::ToolButton(calibrate_menu, FA_UNDO);
			b_reset_calibration->set_callback([axis_ptr, b_reset_calibration]() {
				AxisCalibration defaults;
				axis_ptr->apply_calibration(defaults);
				b_reset_calibration->set_pushed(false);
			});

			auto b_more_options = new nanogui::ToolButton(calibrate_menu, FA_COG);
			b_more_options->set_callback([this, axis_ptr, b_more_options]() {
				nanogui::Vector2f old_pos;
				if (popup) {
					old_pos = popup->position();
					screen()->dispose_window(popup);
				}
				popup = new CalibrationPopup(this, axis_ptr);
				popup->set_position(old_pos);
				b_more_options->set_pushed(false);
			});

			auto prog_bar = new nanogui::ProgressBar(axes_table);
			prog_bar->set_fixed_width(150);
			
			axis_table_entries.push_back(TableRow{.bind_name = bind_name, .axis_value = prog_bar});
			axis_num++;
		}
	}
	perform_layout(screen()->nvg_context());
	
}

void ControllerConfig::refresh() {
	int selection_js_id = -1;
	if (device_js_ids.size() > devices_selector->selected_index())
		selection_js_id = device_js_ids[devices_selector->selected_index()];
	device_names.clear();
	device_js_ids.clear();
	for (auto& dev : mgr.devices()) {
		if (dev.present()) {
			device_names.push_back(std::to_string(dev.joystick_id()) + ": " + dev.device_name());
			device_js_ids.push_back(dev.joystick_id());
			// Try to preserve the old selection
			if (dev.joystick_id() == selection_js_id) {
				devices_selector->set_selected_index(device_js_ids.size() - 1);
			}
		}
	}
	if (device_names.empty()) {
		devices_selector->set_enabled(false);
	} else {
		devices_selector->set_enabled(true);
	}
	devices_selector->set_items(device_names);

	recreate_axes_table();

	// if (popup) {
	// 	screen()->dispose_window(popup);
	// 	popup = nullptr;
	// }

	screen()->perform_layout();
}

void ControllerConfig::draw(NVGcontext* ctx) {
	// Check for hardware changes
	if (last_hw_config_idx < mgr.hw_config_idx()) {
		last_hw_config_idx = mgr.hw_config_idx();
		refresh();
	}
	// Update info displays in the table
	if (devices_selector->enabled()) {
		Controller& selected_dev = mgr.devices()[device_js_ids[devices_selector->selected_index()]];
		auto it = axis_table_entries.begin();
		for (auto& axis : selected_dev.axes()) {
			it->axis_value->set_value((axis.value() - JoystickAxis::AXIS_MIN) / JoystickAxis::AXIS_RANGE);
			it->bind_name->set_caption(axis.action().name);
			++it;
		}
	}
	nanogui::Window::draw(ctx);
}

ControllerConfig::CalibrationPopup::CalibrationPopup(ControllerConfig* p, JoystickAxis* a) :
		nanogui::Window(p->screen(), "Detailed Calibration"),
		axis(a) {

	set_layout(new nanogui::GroupLayout());

	/*
		Window Section: Calibrated value
		Bar and textbox showing the calibrated joystick axis input
	*/
	new nanogui::Label(this, "Calibrated value", "sans-bold", 18);

	auto calibrated_values = new nanogui::Widget(this);
	calibrated_values->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 6));

	cal_value_bar = new nanogui::ProgressBar(calibrated_values);
	cal_value_bar->set_fixed_width(100);
	cal_value_box = new nanogui::TextBox(calibrated_values);
	cal_value_box->set_fixed_width(80);
	cal_value_box->set_font_size(18);

	/*
		Window Section: Raw value
		Bar and textbox showing the uncalibrated axis input
	*/

	new nanogui::Label(this, "Raw value", "sans-bold", 18);

	auto raw_values = new nanogui::Widget(this);
	raw_values->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 6));

	raw_value_bar = new nanogui::ProgressBar(raw_values);
	raw_value_bar->set_fixed_width(100);
	raw_value_box = new nanogui::TextBox(raw_values);
	raw_value_box->set_fixed_width(80);
	raw_value_box->set_font_size(18);

	/*
		Window Section: Calibration
		Slider bars and textboxes for manually adjusting the calibration
	*/

	const int slider_width = 180;
	const int textbox_width = 120;

	new nanogui::Label(this, "Calibration", "sans-bold", 18);

	auto cal_params = new nanogui::Widget(this);
	cal_params->set_layout(new nanogui::GridLayout(nanogui::Orientation::Horizontal, 3, nanogui::Alignment::Middle, 0, 12));

	/*
		Window Sub-Section: Axis Center
		Label, slider, and textbox
	*/

	new nanogui::Label(cal_params, "Center");
	center.slider = new nanogui::Slider(cal_params);
	center.slider->set_value(to_percent_scale(axis->calibration().center));
	center.slider->set_fixed_width(slider_width);
	center.slider->set_callback([this](float x) {
		x = to_axis_scale(x);
		x = fmin(fmax(x, -0.99F), 0.99F); 
		axis->calibration().set_center(x);
	});

	center.text = new nanogui::TextBox(cal_params);
	center.text->set_editable(true);
	center.text->set_fixed_width(textbox_width);
	center.text->set_value(std::to_string(axis->calibration().center));

	center.text->set_callback([this](const std::string& str) {
		try {
			float x = std::stof(str);
			axis->calibration().set_center(x);
		} catch (const std::logic_error&) {
			center.text->set_value(std::to_string(axis->calibration().center));
		}
		return true;
	});

	/*
		Window Sub-Section: Axis Dead Zone
		Label, slider, and textbox
	*/

	new nanogui::Label(cal_params, "Dead Zone");
	deadzone.slider = new nanogui::Slider(cal_params);
	deadzone.slider->set_value(axis->calibration().dead_zone);
	deadzone.slider->set_fixed_width(slider_width);
	deadzone.slider->set_callback([this](float x) {
		axis->calibration().set_dead_zone(x);
	});

	deadzone.text = new nanogui::TextBox(cal_params);
	deadzone.text->set_editable(true);
	deadzone.text->set_fixed_width(textbox_width);
	deadzone.text->set_value(std::to_string(axis->calibration().dead_zone * 100.0F));
	deadzone.text->set_units("%");
	deadzone.text->set_callback([this](const std::string& str) {
		try {
			float x = std::stof(str);
			axis->calibration().set_dead_zone(x / 100.0F);
		} catch (const std::logic_error&) {
			deadzone.text->set_value(std::to_string(axis->calibration().dead_zone * 100.0F));
		}
		return true;
	});

	/*
		Window Sub-Section: Axis Min
		Label, slider, and textbox
	*/

	new nanogui::Label(cal_params, "Minimum");
	minimum.slider = new nanogui::Slider(cal_params);
	minimum.slider->set_value(to_percent_scale(axis->calibration().min));
	minimum.slider->set_fixed_width(slider_width);
	minimum.slider->set_callback([this](float x) {
		x = to_axis_scale(x);
		AxisCalibration c = axis->calibration();
		c.min = x;
		axis->apply_calibration(c);
	});

	minimum.text = new nanogui::TextBox(cal_params);
	minimum.text->set_editable(true);
	minimum.text->set_fixed_width(textbox_width);
	minimum.text->set_value(std::to_string(axis->calibration().min));

	minimum.text->set_callback([this](const std::string& str) {
		try {
			float x = std::stof(str);
			if (x > 1.0F || x < 1.0F) throw std::logic_error("invalid axis min");
			AxisCalibration c = axis->calibration();
			c.min = x;
			axis->apply_calibration(c);
		} catch (const std::logic_error&) {
			minimum.text->set_value(std::to_string(axis->calibration().min));
		}
		return true;
	});

	/*
		Window Sub-Section: Axis Max
		Label, slider, and textbox
	*/

	new nanogui::Label(cal_params, "Maximum");
	maximum.slider = new nanogui::Slider(cal_params);
	maximum.slider->set_value(to_percent_scale(axis->calibration().max));
	maximum.slider->set_fixed_width(slider_width);
	maximum.slider->set_callback([this](float x) {
		x = to_axis_scale(x);
		AxisCalibration c = axis->calibration();
		c.max = x;
		axis->apply_calibration(c);
	});

	maximum.text = new nanogui::TextBox(cal_params);
	maximum.text->set_editable(true);
	maximum.text->set_fixed_width(textbox_width);
	maximum.text->set_value(std::to_string(axis->calibration().max));
	maximum.text->set_callback([this](const std::string& str) {
		try {
			float x = std::stof(str);
			if (x > 1.0F || x < 1.0F) throw std::logic_error("invalid axis max");
			AxisCalibration c = axis->calibration();
			c.max = x;
			axis->apply_calibration(c);
		} catch (const std::logic_error&) {
			maximum.text->set_value(std::to_string(axis->calibration().max));
		}
		return true;
	});

	auto b_close = new nanogui::Button(this, "Close", FA_WINDOW_CLOSE);
	b_close->set_callback([this]() {
		screen()->dispose_window(this);
	});

	screen()->perform_layout();
	
}

void ControllerConfig::CalibrationPopup::draw(NVGcontext* ctx) {

	if (axis) {
	
		float x = (axis->value() - JoystickAxis::AXIS_MIN) / JoystickAxis::AXIS_RANGE;
		cal_value_bar->set_value(x);

		cal_value_box->set_value(std::to_string(axis->value()));

		x = (axis->raw_value() - JoystickAxis::AXIS_MIN) / JoystickAxis::AXIS_RANGE;
		raw_value_bar->set_value(x);
		raw_value_box->set_value(std::to_string(axis->raw_value()));

	}

	nanogui::Window::draw(ctx);
}
