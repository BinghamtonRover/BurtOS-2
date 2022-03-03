#pragma once

#include <widgets/window.hpp>
#include <string>
#include <events.hpp>

namespace gui {

class ElectricalInfo : public gui::Window {
	public:
		ElectricalInfo(nanogui::Widget* parent);

		virtual nanogui::Vector2i preferred_size(NVGcontext* ctx) const override;
	private:
		class SensorDisplay {
		public:
			SensorDisplay(nanogui::Widget* parent, const std::string& name);
			SensorDisplay() {}
			
			void create(nanogui::Widget* parent, const std::string& name, const std::string& initv = "<unset>");

			template<typename T>
			void create(nanogui::Widget* parent, const std::string& name, const T& initv) {
				create(parent, name, std::to_string(initv));
			}

			void set_value(const std::string& val);

			template<typename T>
			void set_value(const T& val) {
				set_value(std::to_string(val));
			}

			nanogui::Widget* container;
			nanogui::Label* label;
			nanogui::Label* value;

		};
		void add_sect_label(const std::string& lbl);
		SensorDisplay bat_voltage;
		SensorDisplay bat_current;

		SensorDisplay sv12_voltage;
		SensorDisplay sv12_current;
		SensorDisplay sv12_temp;

		SensorDisplay sv5_voltage;
		SensorDisplay sv5_current;
		SensorDisplay sv5_temp;

		SensorDisplay odrive0_current;
		SensorDisplay odrive1_current;
		SensorDisplay odrive2_current;

		event::Handler battery_event;
		event::Handler supply12v_event;
		event::Handler supply5v_event;
		event::Handler odrive_event;
		
};

}
