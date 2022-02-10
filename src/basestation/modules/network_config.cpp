#include <modules/network_config.hpp>
#include <basestation.hpp>

#include <limits>
#include <string>

#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/messagedialog.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/icons.h>

#include <widgets/functionbox.hpp>

NetworkConfig::NetworkConfig(nanogui::Screen* screen) :
	nanogui::Window(screen, "Network Settings") {

	auto layout = new nanogui::AdvancedGridLayout({10, 0, 10, 0}, {});
	layout->set_margin(10);
	layout->set_col_stretch(2, 1);
	m_layout = layout;

	nanogui::FormHelper* form = new nanogui::FormHelper(screen);
	form->set_window(this);


	form->add_group("Subsystem Link");
	ip_entry = form->add_variable("IP Address", ip_str);

	// This terrifying regex for IP validation was provided by:
	// https://stackoverflow.com/a/36760050
	ip_entry->set_format("^((25[0-5]|(2[0-4]|1\\d|[1-9]|)\\d)(\\.(?!$)|$)){4}$");
	ip_entry->set_value(Basestation::get().get_subsystem_sender().destination_endpoint().address().to_string());
	ip_entry->set_fixed_width(text_entry_width);
	ip_entry->set_callback([this](const std::string& str) {
		net::MessageSender& subsys = Basestation::get().get_subsystem_sender();
		if (str.size() != 0) {
			try {

				auto ep = subsys.destination_endpoint();
				ep.address(boost::asio::ip::address_v4::from_string(str));
				subsys.set_destination_endpoint(ep);
				return true;
			} catch (const std::exception& err) {
				new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Warning, "Unexpected Error", err.what());
			}
		}
		ip_str = subsys.destination_endpoint().address().to_string();
		return false;
	});

	port = Basestation::get().get_subsystem_sender().destination_endpoint().port();
	port_entry = form->add_variable("Port", port);
	// Regex: https://3widgets.com/, range: 0 - 65535
	port_entry->set_format("(\\d|[1-9]\\d{1,3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])");
	port_entry->set_fixed_width(text_entry_width);
	port_entry->set_alignment(nanogui::TextBox::Alignment::Right);
	port_entry->set_callback([this](const std::string& str) {
		net::MessageSender& subsys = Basestation::get().get_subsystem_sender();
		if (str.size() != 0) {
			try {
				int set_port = std::stoi(str);
				if (set_port >= 0 && set_port <= std::numeric_limits<uint16_t>().max()) {
					auto ep = subsys.destination_endpoint();
					ep.port(set_port);
					subsys.set_destination_endpoint(ep);
					return true;
				}
			} catch (const std::logic_error&) {}
		}
		port = subsys.destination_endpoint().port();
		return false;
	});

	enable = Basestation::get().get_subsystem_sender().enabled();
	auto enable_box = form->add_variable("Open", enable);
	enable_box->set_callback([](bool checked) {
		if (checked)
			Basestation::get().get_subsystem_sender().enable();
		else
			Basestation::get().get_subsystem_sender().disable();
	});

	interval = Basestation::get().get_remote_drive_controller().get_interval();
	auto interval_entry = form->add_variable("Movement Update Interval", interval);
	interval_entry->set_format("\\d*");
	interval_entry->set_units("millis");
	interval_entry->set_callback([this](int new_interval) {
		if (new_interval >= 0) {
			Basestation::get().get_remote_drive_controller().set_interval(new_interval);
			return true;
		}
		interval = Basestation::get().get_remote_drive_controller().get_interval();
		return false;
	});

	form->add_button("Close", [this] {
		this->screen()->dispose_window(this);
	});

	set_position(15);
	set_visible(true);
	screen->perform_layout();
}