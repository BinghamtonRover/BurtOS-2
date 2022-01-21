#include "lua_modules.hpp"

#include "session.hpp"

#include <sstream>

const struct luaL_Reg lua_ctrl_lib::lib[] = {
	{"show_devices", lua_ctrl_lib::show_devices},
	{"show_actions", lua_ctrl_lib::show_actions},
	{"show_axes", lua_ctrl_lib::show_axes},
	{"bind", lua_ctrl_lib::bind},
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
		listing << c.name << "\n";
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
				<< "), action = " << axis.action().name << "\n"; 
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
