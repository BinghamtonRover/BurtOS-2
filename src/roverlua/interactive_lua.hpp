/*
	Launch a Lua prompt in "interactive" mode for the Console module.
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

#include "rover_lua.hpp"
#include <functional>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <mutex>
#include <condition_variable>

namespace rover_lua {

class InteractivePrompt {
private:
	constexpr static std::size_t LUA_MAXINPUT = 512;
	lua_State* L;
	void* _custom_ptr;
	std::function<void(const char*)> _c_write_line;
	std::mutex execute_stream_lock;
	std::condition_variable cv_line_available;
	std::string line_in_unread;
	std::string line_in;
	bool should_interrupt = false;

	// source: lua.c
	int report_error(int status);
	int docall(int narg, int nres);
	void lua_print();
	int incomplete(int status);
	int multiline();
	int addreturn();
	const char *get_prompt(int firstline);
	int pushline(int firstline);
	int loadline();
	static int msghandler(lua_State* L);
	// end lua.c

	// Block while waiting line_in to be updated
	void get_input();

	static void check_interrupt(lua_State* L, lua_Debug* db);

public:
	// Lua functions built-in to the interactive prompt
	struct Builtin {
		static int print(lua_State* L);
	};
	// C functions which provide the default action when built-in commands run
	struct Default {
		static void write_line(const char* str);
	};

	InteractivePrompt();
	~InteractivePrompt();

	void execute_line(const std::string&);
	// Run Lua interpreter. Blocking call.
	void run();
	inline void interrupt() { should_interrupt = true; }
	inline void set_write_line_callback(const std::function<void(const char*)>& f) { _c_write_line = f; }
	inline lua_State* lua() { return L; }

	template<typename T>
	void save_instance(T& instance) {
		_custom_ptr = &instance;
	}

	template<typename T>
	static T& get_instance(lua_State* L) {
		auto self = reinterpret_cast<InteractivePrompt*>(rover_lua::get_custom_ptr(L));
		return *reinterpret_cast<T*>(self->_custom_ptr);
	}

};

}
