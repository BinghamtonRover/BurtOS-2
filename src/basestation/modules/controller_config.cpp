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

	auto close_button = new nanogui::Button(this, "Close");
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
			new nanogui::Label(axes_table, "unimplemented");
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
