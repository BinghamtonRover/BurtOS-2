#include "controller_config.hpp"

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

	popup = new CalibrationPopup(screen, mgr);
	popup->set_visible(false);
	popup->set_position(screen->size() / 3);

	bind_popup = new BindingPopup(screen, mgr);
	bind_popup->set_visible(false);
	bind_popup->set_position(screen->size() / 3);

	this->screen()->perform_layout();
	
}

ControllerConfig::~ControllerConfig() {
	auto s = screen();
	s->dispose_window(popup);
	s->dispose_window(bind_popup);
}

void ControllerConfig::recreate_axes_table() {
	axis_table_entries.clear();
	// Delete all entries currently in the table and recreate below
	while (axes_table->child_count()) {
		axes_table->remove_child_at(0);
	}
	const char* col_title_font = "sans-bold";
	int col_title_size = 24;

	// Column titles: axes_table uses a 4-column grid layout that fills top to bottom, left to right
	new nanogui::Label(axes_table, "Axis", col_title_font, col_title_size);
	new nanogui::Label(axes_table, "Binding", col_title_font, col_title_size);
	new nanogui::Label(axes_table, "Calibrate", col_title_font, col_title_size);
	new nanogui::Label(axes_table, "Value", col_title_font, col_title_size);

	if (devices_selector->enabled()) {
		Controller& selected_dev = mgr.devices()[device_js_ids[devices_selector->selected_index()]];
		// Draw a table entry for each axis
		// Each row has the name, bound function, calibration options, and the input value
		int axis_num = 0;
		for (auto& axis : selected_dev.axes()) {
			if (selected_dev.is_gamepad()) {
				new nanogui::Label(axes_table, gamepad::AXES[axis_num].DISPLAY_NAME);
			} else {
				new nanogui::Label(axes_table, std::to_string(axis_num));
			}

			// The button callbacks must be able to find their respective JoystickAxis
			// The HW config may change and reallocate between loops
			// Callbacks should not save pointers to individual devices or joysticks
			// Instead, capture Joystick ID and Axis #

			int selected_js_id = selected_dev.joystick_id();
			auto bind_name = new nanogui::Button(axes_table, axis.action().name(), FA_EDIT);
			bind_name->set_callback([this, axis_num, selected_js_id]() {
				bind_popup->set_selection(selected_js_id, axis_num);
				bind_popup->set_visible(true);
				bind_popup->request_focus();
			});
			bind_name->set_fixed_width(150);

			// Calibration menu has 3 buttons: start/stop, reset, and open detailed config
			auto calibrate_menu = new nanogui::Widget(axes_table);
			calibrate_menu->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 6));

			auto b_toggle_calibrating = new nanogui::ToolButton(calibrate_menu, FA_PLAY);
			b_toggle_calibrating->set_callback([this, axis_num, selected_js_id, b_toggle_calibrating]() {
				b_toggle_calibrating->set_pushed(false);

				JoystickAxis& target = mgr.devices().at(selected_js_id).axes().at(axis_num);
				// The icon will be wrong if calibration is modified with console, but the action will
				// be correct
				if (target.action().name() == "Calibrating") {
					b_toggle_calibrating->set_icon(FA_PLAY);
					target.end_calibration();
				} else {
					b_toggle_calibrating->set_icon(FA_STOP);
					target.start_calibration();
				}
			});

			auto b_reset_calibration = new nanogui::ToolButton(calibrate_menu, FA_UNDO);
			b_reset_calibration->set_callback([this, axis_num, selected_js_id, b_reset_calibration]() {
				b_reset_calibration->set_pushed(false);
				JoystickAxis& target = mgr.devices().at(selected_js_id).axes().at(axis_num);
				AxisCalibration defaults;
				target.apply_calibration(defaults);
			});

			auto b_more_options = new nanogui::ToolButton(calibrate_menu, FA_COG);
			b_more_options->set_callback([this, axis_num, selected_js_id, b_more_options]() {
				b_more_options->set_pushed(false);
				popup->set_selection(selected_js_id, axis_num);
				popup->set_visible(true);
				popup->request_focus();
			});

			auto prog_bar = new nanogui::ProgressBar(axes_table);
			prog_bar->set_fixed_width(150);
			
			axis_table_entries.push_back(TableRow{bind_name, prog_bar});
			axis_num++;
		}
	}
	perform_layout(screen()->nvg_context());
	
}

void ControllerConfig::refresh() {
	// Save the joystick ID of the old selection so we can reselect it after the refresh
	int selection_js_id = -1;
	int gui_selected_idx = devices_selector->selected_index();
	if (gui_selected_idx >= 0 && device_js_ids.size() > static_cast<unsigned>(gui_selected_idx))
		selection_js_id = device_js_ids[devices_selector->selected_index()];

	// Rebuild the list of names and ids
	device_names.clear();
	device_js_ids.clear();
	for (auto& dev : mgr.devices()) {
		if (dev.present()) {
			device_names.push_back(std::to_string(dev.joystick_id()) + ": " + dev.device_name());
			device_js_ids.push_back(dev.joystick_id());
			// Check if this device was the past selection; if so, reselct it
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
			it->bind_name->set_caption(axis.action().name());
			++it;
		}
	}
	nanogui::Window::draw(ctx);
}
