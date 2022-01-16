#pragma once

#include <nanogui/window.h>
#include <nanogui/textbox.h>
#include <nanogui/textarea.h>
#include <nanogui/vscrollpanel.h>
#include "../widgets/action_textbox.hpp"

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

#include <iostream>

class Console : protected nanogui::Window {
private:
	const int layout_margin = 0;
	const int layout_spacing = 6;
	ActionTextBox* entry;
	nanogui::Button* submit;
	nanogui::Widget* entry_bar;
	nanogui::TextArea* console_out;
	void compute_size() {
		// Button should already be preferred size; only resize the text entry
		int available_w = entry_bar->width() - submit->width() - layout_margin - layout_spacing;
		if (available_w < 0) available_w = 0;
		entry->set_fixed_width(available_w);
	}
	lua_State *L;

	// NOT PERMANENT
	inline static Console* active;

public:
	Console(nanogui::Screen* screen) : 
			nanogui::Window(screen, "Console") {
		
		Console::active = this;

		set_position(nanogui::Vector2i(15, 15));
		set_layout(new nanogui::GroupLayout(6));
		set_fixed_width(500);

		auto scroller = new nanogui::VScrollPanel(this);
		scroller->set_fixed_height(300);
		
		console_out = new nanogui::TextArea(scroller);
		console_out->set_font("mono");

		entry_bar = new Widget(this);
		entry_bar->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Maximum, layout_margin, layout_spacing));

		entry = new ActionTextBox(entry_bar, "");
		entry->set_placeholder("");
		entry->set_editable(true);
		entry->set_alignment(nanogui::TextBox::Alignment::Left);
		entry->set_action_callback([this] {
			console_out->append_line("> " + entry->value());

			int error = luaL_loadbuffer(L, entry->value().data(), entry->value().size(), entry->value().data())
					|| lua_pcall(L, 0, 0, 0);
			
			if (error) {
				console_out->append_line(lua_tostring(L, -1));
				lua_pop(L, 1);
			}

			entry->set_value("");
		});

		submit = new nanogui::Button(entry_bar, "Run");
		submit->set_callback([this] {
			entry->action();
		});

		perform_layout(screen->nvg_context());

		compute_size();

		L = luaL_newstate();
		luaL_openlibs(L);
		
		console_out->append_line(LUA_COPYRIGHT);

		lua_pushcfunction(L, console_print);
		lua_setglobal(L, "cprint");

		lua_pushcfunction(L, console_clear);
		lua_setglobal(L, "clear");

		const static std::string replace_print = "print = function(...) print_str = \"\" for i,v in ipairs({...}) do print_str = print_str .. tostring(v) if i ~= select(\"#\", ...) then print_str = print_str .. \"\t\" end end cprint(print_str) end";
		int error = luaL_loadbuffer(L, replace_print.data(), replace_print.size(), "line")
		|| lua_pcall(L, 0, 0, 0);
		
		if (error) {
			console_out->append_line(lua_tostring(L, -1));
			lua_pop(L, 1);
		}

	}

	~Console() {
		lua_close(L);
	}

	static int console_print(lua_State* L) {
		const char* str_out = lua_tostring(L, 1);
		active->console_out->append_line(str_out);
		return 0;
	}

	static int console_clear(lua_State* L) {
		active->console_out->clear();
		return 0;
	}

};
