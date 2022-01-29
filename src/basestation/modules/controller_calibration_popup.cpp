#include "controller_calibration_popup.hpp"

CalibrationPopup::CalibrationPopup(nanogui::Screen* p, ControllerManager& c, int joystick_id, int axis_idx) :
		nanogui::Window(p, "Detailed Calibration"),
		ctrl_manager(c),
		selected_joystick_id(joystick_id),
		selected_axis_idx(axis_idx) {

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
	center.slider->set_fixed_width(slider_width);
	center.slider->set_callback([this](float x) {
		x = JoystickAxis::percent_to_axis(x);
		x = fmin(fmax(x, -0.99F), 0.99F); 
		ctrl_manager.devices().at(selected_joystick_id).axes().at(selected_axis_idx).calibration().set_center(x);
	});

	center.text = new nanogui::TextBox(cal_params);
	center.text->set_editable(true);
	center.text->set_fixed_width(textbox_width);

	center.text->set_callback([this](const std::string& str) {
		try {
			float x = std::stof(str);
			selected_axis().calibration().set_center(x);
		} catch (const std::logic_error&) {
			center.text->set_value(std::to_string(selected_axis().calibration().center));
		}
		return true;
	});

	/*
		Window Sub-Section: Axis Dead Zone
		Label, slider, and textbox
	*/

	new nanogui::Label(cal_params, "Dead Zone");
	deadzone.slider = new nanogui::Slider(cal_params);
	deadzone.slider->set_fixed_width(slider_width);
	deadzone.slider->set_callback([this](float x) {
		selected_axis().calibration().set_dead_zone(x);
	});

	deadzone.text = new nanogui::TextBox(cal_params);
	deadzone.text->set_editable(true);
	deadzone.text->set_fixed_width(textbox_width);
	deadzone.text->set_units("%");
	deadzone.text->set_callback([this](const std::string& str) {
		try {
			float x = std::stof(str);
			selected_axis().calibration().set_dead_zone(x / 100.0F);
		} catch (const std::logic_error&) {
			deadzone.text->set_value(std::to_string(selected_axis().calibration().dead_zone * 100.0F));
		}
		return true;
	});

	/*
		Window Sub-Section: Axis Min
		Label, slider, and textbox
	*/

	new nanogui::Label(cal_params, "Minimum");
	minimum.slider = new nanogui::Slider(cal_params);
	minimum.slider->set_fixed_width(slider_width);
	minimum.slider->set_callback([this](float x) {
		x = JoystickAxis::percent_to_axis(x);
		AxisCalibration c = selected_axis().calibration();
		c.min = x;
		selected_axis().apply_calibration(c);
	});

	minimum.text = new nanogui::TextBox(cal_params);
	minimum.text->set_editable(true);
	minimum.text->set_fixed_width(textbox_width);

	minimum.text->set_callback([this](const std::string& str) {
		try {
			float x = std::stof(str);
			if (x > 1.0F || x < 1.0F) throw std::logic_error("invalid axis min");
			AxisCalibration c = selected_axis().calibration();
			c.min = x;
			selected_axis().apply_calibration(c);
		} catch (const std::logic_error&) {
			minimum.text->set_value(std::to_string(selected_axis().calibration().min));
		}
		return true;
	});

	/*
		Window Sub-Section: Axis Max
		Label, slider, and textbox
	*/

	new nanogui::Label(cal_params, "Maximum");
	maximum.slider = new nanogui::Slider(cal_params);
	maximum.slider->set_fixed_width(slider_width);
	maximum.slider->set_callback([this](float x) {
		x = JoystickAxis::percent_to_axis(x);
		AxisCalibration c = selected_axis().calibration();
		c.max = x;
		selected_axis().apply_calibration(c);
	});

	maximum.text = new nanogui::TextBox(cal_params);
	maximum.text->set_editable(true);
	maximum.text->set_fixed_width(textbox_width);
	maximum.text->set_callback([this](const std::string& str) {
		try {
			float x = std::stof(str);
			if (x > 1.0F || x < 1.0F) throw std::logic_error("invalid axis max");
			AxisCalibration c = selected_axis().calibration();
			c.max = x;
			selected_axis().apply_calibration(c);
		} catch (const std::logic_error&) {
			maximum.text->set_value(std::to_string(selected_axis().calibration().max));
		}
		return true;
	});

	auto b_close = new nanogui::Button(this, "Close", FA_WINDOW_CLOSE);
	b_close->set_callback([this]() {
		set_visible(false);
	});

	screen()->perform_layout();
	
}

void CalibrationPopup::draw(NVGcontext* ctx) {

	if (selected_joystick_id >= 0) {

		try {
			if (!ctrl_manager.devices().at(selected_joystick_id).present()) {
				selected_joystick_id = -1;
				set_visible(false);
			}
			JoystickAxis& axis = selected_axis();

			cal_value_bar->set_value(JoystickAxis::axis_to_percent(axis.value()));
			cal_value_box->set_value(std::to_string(axis.value()));

			raw_value_bar->set_value(JoystickAxis::axis_to_percent(axis.raw_value()));
			raw_value_box->set_value(std::to_string(axis.raw_value()));

			// Update textboxes/sliders to reflect the actual values
			// Do not do this if the widget is focused

			AxisCalibration& c = axis.calibration();
			if (!center.slider->focused()) {
				center.slider->set_value(JoystickAxis::axis_to_percent(c.center));
			}
			if (!deadzone.slider->focused()) {
				deadzone.slider->set_value(c.dead_zone);
			}
			if (!minimum.slider->focused()) {
				minimum.slider->set_value(JoystickAxis::axis_to_percent(c.min));
			}
			if (!maximum.slider->focused()) {
				maximum.slider->set_value(JoystickAxis::axis_to_percent(c.max));
			}
			if (!center.text->focused()) {
				center.text->set_value(std::to_string(c.center));
			}
			if (!deadzone.text->focused()) {
				deadzone.text->set_value(std::to_string(c.dead_zone * 100.0F));
			}
			if (!minimum.text->focused()) {
				minimum.text->set_value(std::to_string(c.min));
			}
			if (!maximum.text->focused()) {
				maximum.text->set_value(std::to_string(c.max));
			}

		} catch (const std::out_of_range&) {
			selected_joystick_id = -1;
			set_visible(false);
		}

	}

	nanogui::Window::draw(ctx);
}

JoystickAxis& CalibrationPopup::selected_axis() {
	return ctrl_manager.devices().at(selected_joystick_id).axes().at(selected_axis_idx);
}