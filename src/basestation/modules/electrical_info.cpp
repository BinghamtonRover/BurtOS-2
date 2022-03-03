#include <modules/electrical_info.hpp>

#include <nanogui/nanogui.h>
#include <rover_control.hpp>
#include <basestation.hpp>

#include <widgets/layouts/simple_column.hpp>
#include <widgets/layouts/simple_row.hpp>

gui::ElectricalInfo::ElectricalInfo(nanogui::Widget* parent) :
	gui::Window(parent, "Electrical", true) {

	set_layout(new gui::SimpleColumnLayout(4, 4, 4, gui::SimpleColumnLayout::HorizontalAnchor::STRETCH));

	rc::Sensor& s = Basestation::get().remote_sensors();

	add_sect_label("Battery");
	bat_voltage.create(this, "Voltage", s.get_battery_voltage());
	bat_current.create(this, "Current", s.get_battery_current());

	add_sect_label("12V Power Supply");
	sv12_voltage.create(this, "Voltage", s.get_v12_supply_voltage());
	sv12_current.create(this, "Current", s.get_v12_supply_current());
	sv12_temp.create(this, "Temperature", s.get_v12_supply_temperature());

	add_sect_label("5V Power Supply");
	sv5_voltage.create(this, "Voltage", s.get_v5_supply_voltage());
	sv5_current.create(this, "Current", s.get_v5_supply_current());
	sv5_temp.create(this, "Temperature", s.get_v5_supply_temperature());

	add_sect_label("ODrives");
	odrive0_current.create(this, "1", s.get_odrive0_current());
	odrive1_current.create(this, "2", s.get_odrive1_current());
	odrive2_current.create(this, "3", s.get_odrive2_current());

	set_size(nanogui::Vector2i(400, 400));
	parent->perform_layout(screen()->nvg_context());

	battery_event.subscribe(s.EVENT_BATTERY_SENSOR, [this] (float v, float c) {
		bat_voltage.set_value(v);
		bat_current.set_value(c);
	});
	supply12v_event.subscribe(s.EVENT_POWERSUPPLY12V_SENSOR, [this] (float v, float c, float t) {
		sv12_voltage.set_value(v);
		sv12_current.set_value(c);
		sv12_temp.set_value(t);
	});
	supply5v_event.subscribe(s.EVENT_POWERSUPPLY12V_SENSOR, [this] (float v, float c, float t) {
		sv5_voltage.set_value(v);
		sv5_current.set_value(c);
		sv5_temp.set_value(t);
	});
	odrive_event.subscribe(s.EVENT_ODRIVE_SENSOR, [this] (float c0, float c1, float c2) {
		odrive0_current.set_value(c0);
		odrive1_current.set_value(c1);
		odrive2_current.set_value(c2);
	});

}

void gui::ElectricalInfo::add_sect_label(const std::string& lbl) {
	auto label = new nanogui::Label(this, lbl, "sans-bold", 18);
	label->set_fixed_size(label->preferred_size(screen()->nvg_context()));
}

nanogui::Vector2i gui::ElectricalInfo::preferred_size(NVGcontext* ctx) const {
	return nanogui::Vector2i(
		m_fixed_size.x() ? m_fixed_size.x() : m_size.x(),
		m_fixed_size.y() ? m_fixed_size.y() : m_size.y()
	);
}

void gui::ElectricalInfo::SensorDisplay::create(nanogui::Widget* parent, const std::string& name, const std::string& initv) {
	container = new nanogui::Widget(parent);
	container->set_layout(new gui::SimpleRowLayout(8, 4));

	label = new nanogui::Label(container, name);
	value = new nanogui::Label(container, initv);
}

gui::ElectricalInfo::SensorDisplay::SensorDisplay(nanogui::Widget* parent, const std::string& name) {
	create(parent, name);
}

void gui::ElectricalInfo::SensorDisplay::set_value(const std::string& val) {
	value->set_caption(val);
}
