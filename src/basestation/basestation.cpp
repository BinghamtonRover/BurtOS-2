#include <basestation.hpp>
#include <modules/console.hpp>
#include <modules/network_settings.hpp>
#include <modules/drive_stats.hpp>
#include <modules/input_config/controller_config.hpp>
#include <controls/lua_ctrl_lib.hpp>

#include <stdexcept>

#include <nanogui/opengl.h>

Basestation* Basestation::main_instance = nullptr;

Basestation::Basestation() : Basestation(boost::property_tree::ptree()) {}

Basestation::Basestation(const boost::property_tree::ptree& config)
	: m_subsystem_sender(main_thread_ctx),
	m_subsystem_feed(main_thread_ctx),
	m_remote_drive(m_subsystem_sender) {

	if (main_instance != nullptr) {
		throw std::runtime_error("Basestation::Basestation: duplicate instance not allowed");
	}
	main_instance = this;

	controller_mgr.init();
	m_remote_drive.add_controller_actions(controller_mgr);
	for (auto& dev : controller_mgr.devices()) {
		if (dev.present() && dev.is_gamepad()) {
			dev.get_gamepad_axis(gamepad::right_trigger).set_action(controller_mgr.find_action("Accelerate"));
			dev.get_gamepad_axis(gamepad::left_x).set_action(controller_mgr.find_action("Steer (180)"));
			dev.get_gamepad_axis(gamepad::left_trigger).set_action(controller_mgr.find_action("Reverse"));
		}
	}
	m_remote_drive.register_listen_handlers(m_subsystem_feed);
	m_remote_sensors.register_listen_handlers(m_subsystem_feed);

	Console::add_setup_routine([](Console& new_console) {
		new_console.load_library("ctrl", lua_ctrl_lib::open);
		new_console.load_library("bs", lua_basestation_lib::open);
	});

	log_sender_error.subscribe(m_subsystem_sender.event_send_error(), [](const boost::system::error_code& ec) {
		static int last_code = boost::system::errc::success;
		static std::chrono::steady_clock::time_point last_reported_err{};

		if (last_code != ec.value() || std::chrono::duration<double>(std::chrono::steady_clock::now() - last_reported_err).count() >= 1.0) {
			std::cerr << "Error sending to subsystem: " << ec.message() << std::endl;

			last_code = ec.value();
			last_reported_err = std::chrono::steady_clock::now();
		}
	});
	log_feed_error.subscribe(m_subsystem_feed.event_receive_error(), [](const boost::system::error_code& ec) {
		static int last_code = boost::system::errc::success;
		static std::chrono::steady_clock::time_point last_reported_err{};

		if (last_code != ec.value() || std::chrono::duration<double>(std::chrono::steady_clock::now() - last_reported_err).count() >= 1.0) {
			std::cerr << "Error listening to subsystem feed: " << ec.message() << std::endl;

			last_code = ec.value();
			last_reported_err = std::chrono::steady_clock::now();
		}
	});

	read_settings(config);

}

Basestation::~Basestation() {
	main_instance = nullptr;
}

void Basestation::read_settings(const boost::property_tree::ptree& settings_tree) {
	// network settings
	auto net_cfg = settings_tree.get_child_optional("network");
	if (net_cfg) {

		auto subsys_feed_cfg = net_cfg.get().get_child_optional("subsystem_feed");
		if (subsys_feed_cfg) {
			bool enable = subsys_feed_cfg.get().get<bool>("enable", true);
			bool use_multicast = subsys_feed_cfg.get().get<bool>("multicast", true);
			boost::optional port = subsys_feed_cfg.get().get_optional<uint16_t>("port");
			boost::optional ip = subsys_feed_cfg.get().get_optional<std::string>("addr");

			// If the IP is not specified, assume multicast should be disabled since it requires both IP and port
			if (!ip)
				use_multicast = false;

			boost::asio::ip::udp::endpoint feed_ep;
			if (ip) {
				try {
					feed_ep.address(boost::asio::ip::address_v4::from_string(ip.get()));
				} catch (const boost::system::system_error& err) {
					std::cerr << "Invalid subsystem feed IP address: " << ip.get() << " (" << err.what() << ")\n";
					enable = false;
				}
			}
			if (port) {
				feed_ep.port(port.get());
			}

			try {
				m_subsystem_feed.set_listen_endpoint(feed_ep);
				m_subsystem_feed.set_multicast(use_multicast);

				if (enable && (ip || !use_multicast) && port && !m_subsystem_feed.opened()) {
					m_subsystem_feed.open();
				}
			} catch (const boost::system::system_error& err) {
				std::cerr << "Invalid configuration for subsystem feed: " << err.what() << "\n";
			}
		}

		auto subsys_ep_cfg = net_cfg.get().get_child_optional("subsystem_endpoint");
		if (subsys_ep_cfg) {
			bool enable = subsys_ep_cfg.get().get<bool>("enable", true);
			auto port = subsys_ep_cfg.get().get_optional<uint16_t>("port");
			auto ip = subsys_ep_cfg.get().get_optional<std::string>("addr");

			boost::asio::ip::udp::endpoint subsys_ep;
			if (ip) {
				try {
					subsys_ep.address(boost::asio::ip::address_v4::from_string(ip.get()));
				} catch (const boost::system::system_error& err) {
					std::cerr << "Invalid subsystem endpoint IP address: " << ip << " (" << err.what() << ")\n";
					enable = false;
				}
			}
			if (port) {
				subsys_ep.port(port.get());
			}
			try {
				if (enable && ip && port) {
					m_subsystem_sender.enable();
				} else {
					m_subsystem_sender.disable();
				}
				m_subsystem_sender.set_destination_endpoint(subsys_ep);
			} catch (const boost::system::system_error& err) {
				std::cerr << "Invalid configuration for subsystem endpoint: " << err.what() << "\n";
			}

		}

		auto movement_interval_ms = net_cfg.get().get("movement_interval_ms", 100);
		m_remote_drive.set_interval(movement_interval_ms);

	}	// end network settings

}

void Basestation::write_settings(boost::property_tree::ptree& to) {
	{
		boost::property_tree::ptree network_cfg;
		{
			boost::property_tree::ptree subsys_feed_cfg;
			subsys_feed_cfg.put("port", m_subsystem_feed.listen_endpoint().port());
			subsys_feed_cfg.put("addr", m_subsystem_feed.listen_endpoint().address().to_string());
			subsys_feed_cfg.put("enable", m_subsystem_feed.opened());
			subsys_feed_cfg.put("multicast", m_subsystem_feed.is_multicast());

			network_cfg.add_child("subsystem_feed", subsys_feed_cfg);
		}
		{
			boost::property_tree::ptree subsys_ep_cfg;
			subsys_ep_cfg.put("port", m_subsystem_sender.destination_endpoint().port());
			subsys_ep_cfg.put("addr", m_subsystem_sender.destination_endpoint().address().to_string());
			subsys_ep_cfg.put("enable", m_subsystem_sender.enabled());

			network_cfg.add_child("subsystem_endpoint", subsys_ep_cfg);
		}
		network_cfg.put("movement_interval_ms", m_remote_drive.get_interval());

		to.add_child("network", network_cfg);
	}
}

void Basestation::add_screen(BasestationScreen* new_scr) {
	screens.push_back(new_scr);
}

BasestationScreen* Basestation::get_focused_screen() const {
	if (screens.size() == 0)
		throw std::runtime_error("Basestation::get_focused_screen: no screens");
	
	// If there's no focused screen, try to use a visible screen
	BasestationScreen* vis = screens[0];
	for (auto& scr : screens) {
		if (scr->visible()) {
			if (scr->focused()) {
				return scr;
			}
			vis = scr;
		}
	}
	return vis;
}

void Basestation::mainloop() {
	while (continue_operating) {
		glfwPollEvents();
		main_thread_ctx.poll();

		m_remote_drive.poll_events();

		controller_mgr.update_controls();

		{
			std::lock_guard lock(schedule_lock);
			for (auto& callback : async_callbacks) {
				callback(*this);
			}
			async_callbacks.clear();
		}

		// Redraw open screens and close others
		// Closed screens will be erased from the vector:
		// On erase, recalculate end, but do not ++it
		auto it = screens.begin();
		auto end = screens.end();
		while (it != end) {
			auto scr = *it;
			if (!scr->visible()) {
				continue;
			} else if (glfwWindowShouldClose(scr->glfw_window())) {
				scr->set_visible(false);
				it = screens.erase(it);
				end = screens.end();
				continue;
			}
			scr->redraw();
			scr->draw_all();

			++it;
		}

		if (screens.size() == 0) {
			break;
		}
	}
}

void Basestation::schedule(const std::function<void(Basestation&)>& callback) {
	std::lock_guard lock(schedule_lock);
	async_callbacks.push_back(callback);
}

const struct luaL_Reg Basestation::lua_basestation_lib::lib[] = {
	{"shutdown", shutdown},
	{"new_screen", new_screen},
	{"new_module", open_module},
	{"set_throttle", set_throttle},
	{"initialize_drive", initialize_drive},
	{NULL, NULL}
};

int Basestation::lua_basestation_lib::set_throttle(lua_State* L) {
	double new_throttle = luaL_checknumber(L, 1);

	if (new_throttle > 0.0F) {
		main_instance->m_remote_drive.set_throttle(new_throttle);
	} else {
		luaL_error(L, "error: throttle must be positive number");
	}

	return 0;
}

int Basestation::lua_basestation_lib::open_module(lua_State* L) {
	const char* name = luaL_checkstring(L, 1);
	int act = 0;
	if (strcmp("net", name) == 0) {
		act = 1;
	} else if (strcmp("ctrl", name) == 0) {
		act = 2;
	} else if (strcmp("console", name) == 0) {
		act = 3;
	} else if (strcmp("drive", name) == 0) {
		act = 4;
	}
	if (act != 0) {
		Basestation::async([act](Basestation& bs) {
			auto s = bs.get_focused_screen();
			nanogui::Window* wnd = nullptr;
			switch (act) {
				case 1:
					wnd = new gui::NetworkSettings(s);
					break;
				case 2:
					wnd = new ControllerConfig(s, bs.controller_manager());
					break;
				case 3:
					wnd = new Console(s);
					break;
				case 4:
					wnd = new gui::DriveStats(s);
					break;
			}
			if (wnd != nullptr) {
				wnd->center();
			}
		});
	} else {
		luaL_error(L, "error: unknown module type");
	}
	return 0;
}

int Basestation::lua_basestation_lib::shutdown(lua_State*) {
	main_instance->continue_operating = false;
	return 0;
}

int Basestation::lua_basestation_lib::new_screen(lua_State*) {
	async([](Basestation& bs) {
		bs.add_screen(new BasestationScreen());
	});
	return 0;
}

void Basestation::lua_basestation_lib::open(lua_State* L) {
	luaL_newlib(L, lib);
}

int Basestation::lua_basestation_lib::initialize_drive(lua_State* L) {
	Basestation::get().remote_drive().set_drive_mode(drive::DriveMode_Mode_CALIBRATING);
	return 0;
}
