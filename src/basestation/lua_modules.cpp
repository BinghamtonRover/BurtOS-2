#include "lua_modules.hpp"

#include "session.hpp"

#include <sstream>

const struct luaL_Reg lua_ctrl_lib::lib[] = {
	{"show_devices", lua_ctrl_lib::show_devices},
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