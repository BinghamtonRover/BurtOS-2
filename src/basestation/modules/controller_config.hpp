#pragma once

#include <vector>
#include <string>
#include <functional>
#include <nanogui/nanogui.h>

#include "../controls/controller_manager.hpp"

/*
	Module for viewing, configuring, and calibrating connected controllers
*/
class ControllerConfig : public nanogui::Window {
	private:
		// Pop-up window with fine details for an axis
		class CalibrationPopup : public nanogui::Window {
			private:
				// Each configurable value will use a slider and textbox
				struct ConfigValue {
					nanogui::Slider* slider;
					nanogui::TextBox* text;
				};
				JoystickAxis* axis;
				nanogui::ProgressBar* cal_value_bar;
				nanogui::TextBox* cal_value_box;
				nanogui::ProgressBar* raw_value_bar;
				nanogui::TextBox* raw_value_box;
				ConfigValue center;
				ConfigValue deadzone;
				ConfigValue minimum;
				ConfigValue maximum;
			public:
				CalibrationPopup(ControllerConfig* parent, JoystickAxis* axis = nullptr);
				virtual void draw(NVGcontext* ctx);
		};
		struct TableRow {
			nanogui::Label* bind_name;
			nanogui::ProgressBar* axis_value;
		};

		ControllerManager& mgr;
		CalibrationPopup* popup = nullptr;
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

		virtual void draw(NVGcontext* ctx);

};
