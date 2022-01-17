/*
	Includes all of the Lua C header files and provides utility functions
*/

/******************************************************************************
* Substantial portions of this software are derived from Lua.org and modified
* to enhance functionality of Binghamton University Rover Team Base Station.
* 
* Please see the copyright notice reproduced below
******************************************************************************/

/******************************************************************************
* Copyright (C) 1994-2022 Lua.org, PUC-Rio.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#pragma once

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

namespace rover_lua {

// Custom pointers: Lua doesn't support many C++ features we rely on, like <functional>
// To gather object-oriented information from static callbacks, use a global mapping of
// lua_State pointers to custom void pointers. Each lua_State can only have 1 custom pointer
//
// get, del, and set provide thread-safety and may be called from any thread at any time

// Allocate or replace the custom pointer for this lua_State
void set_custom_ptr(lua_State* L, void* ptr);

// Deallocate the custom pointer for this lua_State
void del_custom_ptr(lua_State* L);

// Retrieve the custom pointer for this lua_State
void* get_custom_ptr(lua_State* L);

}