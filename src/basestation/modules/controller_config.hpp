#pragma once

#include <vector>
#include <string>
#include <functional>
#include <nanogui/nanogui.h>

#include "../controls/controller_manager.hpp"

class ControllerConfig : public nanogui::Window {
	private:
		struct TableRow {
			nanogui::Label* bind_name;
			nanogui::ProgressBar* axis_value;
		};
		ControllerManager& mgr;
		nanogui::ComboBox* devices_selector;
		nanogui::VScrollPanel* table_scroller;
		nanogui::Widget* axes_table;
		std::vector<TableRow> axis_table_entries;
		std::vector<std::string> device_names;
		std::vector<int> device_js_ids;
		int last_hw_config_idx = 0;

		void recreate_axes_table();
		void refresh();

	public:
		ControllerConfig(nanogui::Screen* screen, ControllerManager& mgr);

		virtual void draw(NVGcontext* ctx);

};
