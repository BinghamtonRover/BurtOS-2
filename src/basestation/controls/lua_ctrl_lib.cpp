#include "lua_ctrl_lib.hpp"

#include <session.hpp>
#include <modules/input_config/controller_config.hpp>

#include <sstream>
#include <future>

const struct luaL_Reg lua_ctrl_lib::lib[] = {
	{"show_devices", lua_ctrl_lib::show_devices},
	{"show_actions", lua_ctrl_lib::show_actions},
	{"show_axes", lua_ctrl_lib::show_axes},
	{"bind", lua_ctrl_lib::bind},
	{"unbind", lua_ctrl_lib::unbind},
	{"start_calibration", lua_ctrl_lib::start_calibration},
	{"end_calibration", lua_ctrl_lib::end_calibration},
	{"set_center", lua_ctrl_lib::set_center},
	{"set_dead_zone", lua_ctrl_lib::set_dead_zone},
	{"menu", lua_ctrl_lib::menu},
	{NULL, NULL}
};

void lua_ctrl_lib::open(lua_State* L) {
	luaL_newlib(L, lib);
}

int lua_ctrl_lib::show_devices(lua_State* L) {

	std::promise<void> p;
	std::future<void> future = p.get_future();

	std::ostringstream listing;
	Session::get_main_session().schedule_sync_event([&p, &listing](Session& s) {
		auto& mgr = s.controller_manager();
		for (auto& dev : mgr.devices()) {
			if (dev.present()) {
				listing << "Joystick " << dev.joystick_id() << ": " << dev.device_name() << " (" << dev.axes().size() << " axes)\n";
			}
		}
		p.set_value();
	});
	future.get();

	lua_getglobal(L, "print");
	lua_pushstring(L, listing.str().c_str());

	if (lua_pcall(L, 1, 0, 0) != 0) {
		luaL_error(L, "error running function 'print': %s", lua_tostring(L, -1));
	}

	return 0;
}

int lua_ctrl_lib::show_actions(lua_State* L) {

	std::promise<void> p;
	std::future<void> f = p.get_future();
	
	std::ostringstream listing;
	Session::get_main_session().schedule_sync_event([&p, &listing] (Session& s) {
		ControllerManager& cmgr = s.controller_manager();
		for (auto& c : cmgr.actions()) {
			listing << c.name() << "\n";
		}
		p.set_value();
	});

	f.get();

	lua_getglobal(L, "print");
	lua_pushstring(L, listing.str().c_str());

	if (lua_pcall(L, 1, 0, 0) != 0) {
		luaL_error(L, "error running function 'print': %s", lua_tostring(L, -1));
	}

	return 0;
}

int lua_ctrl_lib::show_axes(lua_State* L) {

	int joystick_id = luaL_checkinteger(L, 1);

	std::promise<void> p;
	std::future<void> f = p.get_future();

	std::ostringstream listing;
	Session::get_main_session().schedule_sync_event([&p, &listing, joystick_id](Session& s) {
		try {
			ControllerManager& cmgr = s.controller_manager();
			int axis_n = 0;
			for (auto& axis : cmgr.devices().at(joystick_id).axes()) {
				listing << "Axis " << axis_n << ": value = " << axis.value()
					<< " (raw = " << axis.raw_value()
					<< "), action = " << axis.action().name() << "\n"; 
				axis_n += 1;
			}
			p.set_value();
		} catch (...) {
			p.set_exception(std::current_exception());
		}
	});

	try {
		f.get();
	} catch (const std::out_of_range&) {
		luaL_error(L, "error: device number out of range");
	}

	lua_getglobal(L, "print");
	lua_pushstring(L, listing.str().c_str());

	if (lua_pcall(L, 1, 0, 0) != 0) {
		luaL_error(L, "error running function 'print': %s", lua_tostring(L, -1));
	}

	return 0;
}

int lua_ctrl_lib::bind(lua_State* L) {
	int joystick_id = luaL_checkinteger(L, 1);
	int axis_n = luaL_checkinteger(L, 2);
	const char* action = luaL_checkstring(L, 3);

	std::promise<void> p;
	std::future<void> f = p.get_future();

	Session::get_main_session().schedule_sync_event([&p, action, joystick_id, axis_n] (Session& s) {
		try {
			ControllerManager& cmgr = s.controller_manager();
			cmgr.devices().at(joystick_id).axes().at(axis_n).set_action(cmgr.find_action(action));
			p.set_value();
		} catch (...) {
			p.set_exception(std::current_exception());
		}
	});

	try {
		f.get();
	} catch (const std::out_of_range&) {
		luaL_error(L, "error: device number or joystick id out of range");
	} catch (const std::invalid_argument&) {
		luaL_error(L, "error: invalid action");
	}
	return 0;
}

int lua_ctrl_lib::unbind(lua_State* L) {
	int joystick_id = luaL_checkinteger(L, 1);
	int axis_n = luaL_checkinteger(L, 2);

	std::promise<void> p;
	std::future<void> f = p.get_future();

	Session::get_main_session().schedule_sync_event([&p, joystick_id, axis_n] (Session& s) {
		try {
			ControllerManager& cmgr = s.controller_manager();
			cmgr.devices().at(joystick_id).axes().at(axis_n).unbind();
			p.set_value();
		} catch (...) {
			p.set_exception(std::current_exception());
		}
	});

	try {
		f.get();
	} catch (const std::out_of_range&) {
		luaL_error(L, "error: device number or joystick id out of range");
	}
	return 0;
}

int lua_ctrl_lib::start_calibration(lua_State* L) {
	int joystick_id = luaL_checkinteger(L, 1);
	int axis_n = luaL_checkinteger(L, 2);

	std::promise<bool> start_promise;
	std::future<bool> start_future = start_promise.get_future();

	Session::get_main_session().schedule_sync_event([joystick_id, axis_n, &start_promise] (Session& s) {
		try {
			ControllerManager& cmgr = s.controller_manager();
			JoystickAxis& ax = cmgr.devices().at(joystick_id).axes().at(axis_n);
			if (ax.action().name() != "Calibrating") {
				ax.start_calibration();
				start_promise.set_value(true);
			} else {
				start_promise.set_value(false);
			}
		} catch (...) {
			start_promise.set_exception(std::current_exception());
		}
	});

	try {
		if (!start_future.get()) {
			luaL_error(L, "error: device already calibrating");
		}
	} catch (const std::out_of_range&) {
		luaL_error(L, "error: device number or joystick id out of range");
	}
	return 0;
}

int lua_ctrl_lib::end_calibration(lua_State* L) {
	int joystick_id = luaL_checkinteger(L, 1);
	int axis_n = luaL_checkinteger(L, 2);

	std::promise<bool> end_promise;
	std::future<bool> end_future = end_promise.get_future();

	Session::get_main_session().schedule_sync_event([joystick_id, axis_n, &end_promise] (Session& s) {
		try {
			ControllerManager& cmgr = s.controller_manager();
			JoystickAxis& ax = cmgr.devices().at(joystick_id).axes().at(axis_n);
			if (ax.action().name() == "Calibrating") {
				ax.end_calibration();
				end_promise.set_value(true);
			} else {
				end_promise.set_value(false);
			}
		} catch (...) {
			end_promise.set_exception(std::current_exception());
		}
	});

	try {
		if (!end_future.get()) {
			luaL_error(L, "error: device not currently calibrating");
		}
	} catch (const std::out_of_range&) {
		luaL_error(L, "error: device number or joystick id out of range");
	}
	return 0;
}

int lua_ctrl_lib::set_dead_zone(lua_State* L) {
	int joystick_id = luaL_checkinteger(L, 1);
	int axis_n = luaL_checkinteger(L, 2);
	float dz = luaL_checknumber(L, 3);

	std::promise<void> set_promise;
	std::future<void> set_future = set_promise.get_future();

	Session::get_main_session().schedule_sync_event([joystick_id, axis_n, dz, &set_promise] (Session& s) {
		try {
			ControllerManager& cmgr = s.controller_manager();
			cmgr.devices().at(joystick_id).axes().at(axis_n).calibration().set_dead_zone(dz);
			set_promise.set_value();
		} catch (...) {
			set_promise.set_exception(std::current_exception());
		}
	});

	try {
		set_future.get();
	} catch (const std::out_of_range&) {
		luaL_error(L, "error: device number or joystick id out of range");
	} catch (const std::invalid_argument&) {
		luaL_error(L, "error: dead zone invalid");
	}
	return 0;
}

int lua_ctrl_lib::set_center(lua_State* L) {
	int joystick_id = luaL_checkinteger(L, 1);
	int axis_n = luaL_checkinteger(L, 2);
	float ctr = luaL_checknumber(L, 3);

	std::promise<void> set_promise;
	std::future<void> future = set_promise.get_future();

	Session::get_main_session().schedule_sync_event([joystick_id, axis_n, ctr, &set_promise] (Session& s) {
		try {
			s.controller_manager().devices().at(joystick_id).axes().at(axis_n).calibration().set_center(ctr);
			set_promise.set_value();
		} catch (...) {
			set_promise.set_exception(std::current_exception());
		}
	});
	try {
		future.get();
	} catch (const std::out_of_range&) {
		luaL_error(L, "error: device number or joystick id out of range");
	} catch (const std::invalid_argument&) {
		luaL_error(L, "error: center invalid");
	}
	return 0;
}

int lua_ctrl_lib::menu(lua_State*) {
	Session::get_main_session().schedule_sync_event([] (Session& s) {
		nanogui::Screen* scr = &s.get_screen();
		auto window = new ControllerConfig(scr, s.controller_manager());

		// Center on screen
		int xpos = std::max(0, (scr->width() - window->width()) / 2);
		int ypos = std::max(0, (scr->height() - window->height()) / 2);
		window->set_position(nanogui::Vector2i(xpos, ypos));

	});
	return 0;
}
