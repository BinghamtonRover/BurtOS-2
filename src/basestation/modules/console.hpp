#pragma once

#include <nanogui/window.h>
#include <nanogui/textbox.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/nanogui.h>
#include "../widgets/functionbox.hpp"
#include "../widgets/textarea.hpp"

#include <interactive_lua.hpp>

#include <iostream>
#include <thread>

class Console : public nanogui::Window, public rover_lua::InteractivePrompt {
protected:
	struct Builtin {
		static int clear(lua_State* L);
		static int exit(lua_State* L);
		static int title(lua_State* L);
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
	bool should_close = false;
	std::thread lua_runtime;

	void compute_size();

public:
	Console(nanogui::Screen* screen);

	virtual void draw(NVGcontext* ctx);

	inline bool closed() { return should_close; }
	inline void close() { should_close = true; }

};
