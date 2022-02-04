#pragma once

#include <vector>
#include <string>
#include <functional>

#include <nanogui/button.h>
#include <nanogui/progressbar.h>
#include <nanogui/combobox.h>
#include <nanogui/vscrollpanel.h>

#include <modules/input_config/controller_calibration_popup.hpp>
#include <modules/input_config/controller_binding_popup.hpp>

#include <controls/controller_manager.hpp>

/*
	Module for viewing, configuring, and calibrating connected controllers
*/
class ControllerConfig : public nanogui::Window {
	private:
		struct TableRow {
			nanogui::Button* bind_name;
			nanogui::ProgressBar* axis_value;
		};

		ControllerManager& mgr;
		CalibrationPopup* popup = nullptr;
		BindingPopup* bind_popup = nullptr;
		nanogui::ComboBox* devices_selector;
		nanogui::VScrollPanel* table_scroller;
		nanogui::Widget* axes_table;
		std::vector<TableRow> axis_table_entries;
		std::vector<std::string> device_names;
		std::vector<int> device_js_ids;
		int last_hw_config_idx = 0;

		void recreate_axes_table();
		void refresh();
		static float to_percent_scale(float axis_value);
		static float to_axis_scale(float percent);

	public:
		ControllerConfig(nanogui::Screen* screen, ControllerManager& mgr);
		~ControllerConfig();

		virtual void draw(NVGcontext* ctx);

};
