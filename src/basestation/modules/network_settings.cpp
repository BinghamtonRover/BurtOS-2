#include <modules/network_settings.hpp>
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

gui::NetworkSettings::NetworkSettings(nanogui::Screen* screen) :
	nanogui::Window(screen, "Network Settings") {

	auto layout = new nanogui::AdvancedGridLayout({10, 0, 10, 0}, {});
	layout->set_margin(10);
	layout->set_col_stretch(2, 1);
	m_layout = layout;

	nanogui::FormHelper* form = new nanogui::FormHelper(screen);
	form->set_window(this);

	/*
		Section: Subsystem Link settings
	*/

	form->add_group("Subsystem Update Feed");

	auto feed_ip_entry = form->add_variable("Feed IP Address", mcast_feed_str);
	// This terrifying regex for IP validation was provided by:
	// https://stackoverflow.com/a/36760050
	feed_ip_entry->set_format("^((25[0-5]|(2[0-4]|1\\d|[1-9]|)\\d)(\\.(?!$)|$)){4}$");
	feed_ip_entry->set_value(Basestation::get().subsystem_feed().listen_endpoint().address().to_string());
	feed_ip_entry->set_fixed_width(text_entry_width);
	feed_ip_entry->set_callback([this](const std::string& str) {
		net::MessageReceiver& feed = Basestation::get().subsystem_feed();
		if (str.size() != 0) {
			try {

				auto ep = feed.listen_endpoint();
				ep.address(boost::asio::ip::address_v4::from_string(str));
				feed.subscribe(ep);
				return true;
			} catch (const std::exception& err) {
				new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Warning, "Unexpected Error", err.what());
			}
		}
		mcast_feed_str = feed.listen_endpoint().address().to_string();
		return false;
	});

	mcast_feed_port = Basestation::get().subsystem_feed().listen_endpoint().port();

	auto feed_port_entry = form->add_variable("Feed Port", mcast_feed_port);
	// Regex: https://3widgets.com/, range: 0 - 65535
	feed_port_entry->set_format("(\\d|[1-9]\\d{1,3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])");
	feed_port_entry->set_fixed_width(text_entry_width);
	feed_port_entry->set_callback([this](int set_port) {
		auto& feed = Basestation::get().subsystem_feed();
		if (set_port >= 0 && set_port <= std::numeric_limits<uint16_t>().max()) {
			auto ep = feed.listen_endpoint();
			ep.port(set_port);
			feed.subscribe(ep);
			return true;
		}
		mcast_feed_port = feed.listen_endpoint().port();
		return false;
	});

	
	mcast_enable = Basestation::get().subsystem_feed().opened();
	auto feed_enable_box = form->add_variable("Open", mcast_enable);
	feed_enable_box->set_callback([this](bool checked) {
		try {
			if (checked)
				Basestation::get().subsystem_feed().open();
			else
				Basestation::get().subsystem_feed().close();
		} catch (const boost::system::system_error& err) {
			new nanogui::MessageDialog(this->screen(), nanogui::MessageDialog::Type::Warning, "Error Opening Feed", err.what());
		}
	});

	form->add_group("Subsystem Device Link");
	
	auto ip_entry = form->add_variable("Device IP Address", ip_str);

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
	auto port_entry = form->add_variable("Receiver Port", port);
	// Regex: https://3widgets.com/, range: 0 - 65535
	port_entry->set_format("(\\d|[1-9]\\d{1,3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])");
	port_entry->set_fixed_width(text_entry_width);
	port_entry->set_callback([this](int set_port) {
		net::MessageSender& subsys = Basestation::get().get_subsystem_sender();
		if (set_port >= 0 && set_port <= std::numeric_limits<uint16_t>().max()) {
			auto ep = subsys.destination_endpoint();
			ep.port(set_port);
			subsys.set_destination_endpoint(ep);
			return true;
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

	interval = Basestation::get().remote_drive().get_interval();
	auto interval_entry = form->add_variable("Movement Update Interval", interval);
	interval_entry->set_format("\\d*");
	interval_entry->set_units("millis");
	interval_entry->set_callback([this](int new_interval) {
		if (new_interval >= 0) {
			Basestation::get().remote_drive().set_interval(new_interval);
			return true;
		}
		interval = Basestation::get().remote_drive().get_interval();
		return false;
	});

	/*
		(TODO) Section: Video Link settings
		
		(This section is a placeholder)
	*/
	
	form->add_button("Close", [this] {
		this->screen()->dispose_window(this);
	});

	set_position(15);
	set_visible(true);
	screen->perform_layout();
}
