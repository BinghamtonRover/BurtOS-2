#pragma once

#include <rover_lua.hpp>

#include "controls/controller_manager.hpp"

/*
	Controller library for Lua
*/

namespace lua_ctrl_lib {
	extern const struct luaL_Reg lib[];
	void open(lua_State* L);

	int show_devices(lua_State* L);
	int show_actions(lua_State* L);
	int show_axes(lua_State* L);
	int bind(lua_State* L);
}
