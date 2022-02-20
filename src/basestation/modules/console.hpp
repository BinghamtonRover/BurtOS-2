#pragma once

#include <nanogui/window.h>
#include <nanogui/widget.h>
#include <nanogui/button.h>

#include <widgets/functionbox.hpp>
#include <widgets/textarea.hpp>

#include <interactive_lua.hpp>

#include <iostream>
#include <thread>

class Console : public nanogui::Window, public rover_lua::InteractivePrompt {
protected:
	struct Builtin {
		static int clear(lua_State* L);
		static int exit(lua_State* L);
		static int title(lua_State* L);
		static int set_autoscroll(lua_State* L);
		static int get_autoscroll(lua_State* L);
	};
	const int layout_margin = 0;
	const int layout_spacing = 6;
	FunctionBox* entry;
	nanogui::Button* submit;
	nanogui::Widget* entry_bar;
	TextArea* console_out;
	std::vector<std::string> history;
	std::size_t history_index = 0;
	std::string uncommitted_entry;
	std::thread lua_runtime;

	void compute_size();

	inline static std::vector<std::function<void(Console&)>> global_setup;
	static const struct luaL_Reg term_lib[];
	static int luaopen_term(lua_State*);

public:
	Console(nanogui::Screen* screen);
	~Console();

	virtual void draw(NVGcontext* ctx);
	
	// Add a function to run when creating any new Console
	// Intended for initializing Lua functions and variables 
	inline static void add_setup_routine(const std::function<void(Console&)>& f) { global_setup.push_back(f); }

};
