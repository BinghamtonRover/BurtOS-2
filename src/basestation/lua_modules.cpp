#include "lua_modules.hpp"

#include "session.hpp"
#include "modules/controller_config.hpp"

#include <sstream>

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

	ControllerManager& cmgr = Session::get_main_session().controller_manager();
	std::ostringstream listing;
	for (auto& c : cmgr.devices()) {
		if (c.present()) {
			listing << "Joystick " << c.joystick_id() << ": " << c.device_name() << " (" << c.axes().size() << " axes)\n";
		}
	}

	lua_getglobal(L, "print");
	lua_pushstring(L, listing.str().c_str());

	if (lua_pcall(L, 1, 0, 0) != 0) {
		luaL_error(L, "error running function 'print': %s", lua_tostring(L, -1));
	}

	return 0;
}

int lua_ctrl_lib::show_actions(lua_State* L) {

	ControllerManager& cmgr = Session::get_main_session().controller_manager();
	std::ostringstream listing;
	for (auto& c : cmgr.actions()) {
		listing << c.name() << "\n";
	}

	lua_getglobal(L, "print");
	lua_pushstring(L, listing.str().c_str());

	if (lua_pcall(L, 1, 0, 0) != 0) {
		luaL_error(L, "error running function 'print': %s", lua_tostring(L, -1));
	}

	return 0;
}

int lua_ctrl_lib::show_axes(lua_State* L) {

	ControllerManager& cmgr = Session::get_main_session().controller_manager();

	int joystick_id = luaL_checkinteger(L, 1);

	std::ostringstream listing;
	int axis_n = 0;

	try {
		for (auto& axis : cmgr.devices().at(joystick_id).axes()) {
			listing << "Axis " << axis_n << ": value = " << axis.value()
				<< " (raw = " << axis.raw_value()
				<< "), action = " << axis.action().name() << "\n"; 
			axis_n += 1;
		}
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

	try {
		ControllerManager& cmgr = Session::get_main_session().controller_manager();
		cmgr.devices().at(joystick_id).axes().at(axis_n).set_action(cmgr.find_action(action));
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

	try {
		ControllerManager& cmgr = Session::get_main_session().controller_manager();
		cmgr.devices().at(joystick_id).axes().at(axis_n).unbind();
	} catch (const std::out_of_range&) {
		luaL_error(L, "error: device number or joystick id out of range");
	}
	return 0;
}

int lua_ctrl_lib::start_calibration(lua_State* L) {
	int joystick_id = luaL_checkinteger(L, 1);
	int axis_n = luaL_checkinteger(L, 2);
	try {
		ControllerManager& cmgr = Session::get_main_session().controller_manager();
		cmgr.devices().at(joystick_id).axes().at(axis_n).start_calibration();
	} catch (const std::out_of_range&) {
		luaL_error(L, "error: device number or joystick id out of range");
	}
	return 0;
}

int lua_ctrl_lib::end_calibration(lua_State* L) {
	int joystick_id = luaL_checkinteger(L, 1);
	int axis_n = luaL_checkinteger(L, 2);
	try {
		ControllerManager& cmgr = Session::get_main_session().controller_manager();
		cmgr.devices().at(joystick_id).axes().at(axis_n).end_calibration();
	} catch (const std::out_of_range&) {
		luaL_error(L, "error: device number or joystick id out of range");
	}
	return 0;
}

int lua_ctrl_lib::set_dead_zone(lua_State* L) {
	int joystick_id = luaL_checkinteger(L, 1);
	int axis_n = luaL_checkinteger(L, 2);
	float dz = luaL_checknumber(L, 3);
	try {
		ControllerManager& cmgr = Session::get_main_session().controller_manager();
		cmgr.devices().at(joystick_id).axes().at(axis_n).calibration().set_dead_zone(dz);
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
	try {
		ControllerManager& cmgr = Session::get_main_session().controller_manager();
		cmgr.devices().at(joystick_id).axes().at(axis_n).calibration().set_center(ctr);
	} catch (const std::out_of_range&) {
		luaL_error(L, "error: device number or joystick id out of range");
	} catch (const std::invalid_argument&) {
		luaL_error(L, "error: center invalid");
	}
	return 0;
}

int lua_ctrl_lib::menu(lua_State* L) {
	Session::get_main_session().schedule_sync_event([] (Session& bs_gui) {
		nanogui::Screen* scr = &bs_gui.get_screen();
		auto window = new ControllerConfig(scr, bs_gui.controller_manager());

		// Center on screen
		int xpos = std::max(0, (scr->width() - window->width()) / 2);
		int ypos = std::max(0, (scr->height() - window->height()) / 2);
		window->set_position(nanogui::Vector2i(xpos, ypos));

	});
	return 0;
}
