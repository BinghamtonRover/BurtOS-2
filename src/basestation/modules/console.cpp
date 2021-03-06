#include "console.hpp"

#include <nanogui/vscrollpanel.h>
#include <nanogui/layout.h>
#include <nanogui/screen.h>

#include <widgets/layouts/simple_row.hpp>
#include <widgets/layouts/simple_column.hpp>

#include <basestation.hpp>

/*
	Console Built-In Definitions
*/

int Console::Builtin::clear(lua_State* L) {
	auto& self = rover_lua::InteractivePrompt::get_instance<Console>(L);
	self.console_out->clear();
	return 0;
}
int Console::Builtin::exit(lua_State* L) {
	auto& self = rover_lua::InteractivePrompt::get_instance<Console>(L);
	self.stop();

	Basestation::async([&self] (Basestation&) {
		self.screen()->dispose_window(&self);
	});
	return 0;
}
int Console::Builtin::title(lua_State* L) {
	auto& self = rover_lua::InteractivePrompt::get_instance<Console>(L);
	const char* new_title = luaL_checkstring(L, 1);
	self.set_title(new_title);
	return 0;
}
int Console::Builtin::set_autoscroll(lua_State* L) {
	auto& self = rover_lua::InteractivePrompt::get_instance<Console>(L);
	bool enable = lua_toboolean(L, 1);
	self.console_out->set_autoscroll(enable);
	return 0;
}
int Console::Builtin::get_autoscroll(lua_State* L) {
	auto& self = rover_lua::InteractivePrompt::get_instance<Console>(L);
	lua_pushboolean(L, self.console_out->autoscroll());
	return 1;
}

/*
	Console Definitions
*/

void Console::draw(NVGcontext* ctx) {
		if (!active()) {

			entry->focus_event(false);
			entry->set_editable(false);
			submit->set_enabled(false);

		}
		gui::Window::draw(ctx);	
}

const struct luaL_Reg Console::term_lib[] = {
	{"clear", Console::Builtin::clear},
	{"title", Console::Builtin::title},
	{"exit", Console::Builtin::exit},
	{"set_autoscroll", Console::Builtin::set_autoscroll},
	{"get_autoscroll", Console::Builtin::get_autoscroll},
	{NULL, NULL}
};

int Console::luaopen_term(lua_State* L) {
	luaL_newlib(L, term_lib);
	return 1;
}

nanogui::Vector2i Console::preferred_size(NVGcontext* ctx) const {
	return nanogui::Vector2i(
		m_fixed_size.x() ? m_fixed_size.x() : m_size.x(),
		m_fixed_size.y() ? m_fixed_size.y() : m_size.y()
	);
}

Console::Console(nanogui::Screen* screen) : 
		gui::Window(screen, "Console", true),
		lua_runtime(&rover_lua::InteractivePrompt::run_paused, static_cast<rover_lua::InteractivePrompt*>(this)) {

	set_position(nanogui::Vector2i(15, 15));
	set_layout(new gui::SimpleColumnLayout(6, 6, 6, gui::SimpleColumnLayout::HorizontalAnchor::STRETCH));
	set_width(500);
	set_height(400);

	auto scroller = new nanogui::VScrollPanel(this);
	
	console_out = new TextArea(scroller);
	console_out->set_font("mono");

	entry_bar = new Widget(this);
	entry_bar->set_layout(new gui::SimpleRowLayout());

	entry = new FunctionBox(entry_bar, "");
	entry->set_placeholder("");
	entry->set_editable(true);
	entry->set_alignment(nanogui::TextBox::Alignment::Left);
	entry->set_fixed_height(entry->preferred_size(screen->nvg_context()).y());
	entry->set_action_callback([this] (FunctionBox::Action a) {
		switch (a) {
			case FunctionBox::Action::SUBMIT:
				console_out->append_line("> " + entry->value());

				if (!entry->value().empty()) {
					execute_line(entry->value());
					history.push_back(entry->value());
					// Index intentionally beyond last element
					history_index = history.size();
				}

				entry->set_value("");
				break;
			case FunctionBox::Action::SCROLL_UP:
				if (history_index > 0 && !history.empty()) {
					if (history_index == history.size()) {
						uncommitted_entry = entry->value();
					}
					history_index--;
					entry->set_value(history[history_index]);
				}
				break;
			case FunctionBox::Action::SCROLL_DOWN:
				if (history_index + 1 < history.size()) {
					history_index++;
					entry->set_value(history[history_index]);
				} else if (history_index != history.size()) {
					history_index = history.size();
					entry->set_value(uncommitted_entry);
				}
				break;
			case FunctionBox::Action::CANCEL: 
				if (entry->value().empty()) {
					interrupt();
				} else {
					entry->set_value("");
				}
				break;
			default:
				break;
		}

	});

	submit = new nanogui::Button(entry_bar, "Run");
	submit->set_fixed_size(submit->preferred_size(screen->nvg_context()));
	submit->set_callback([this] {
		entry->action(FunctionBox::Action::SUBMIT);
	});
	entry_bar->set_fixed_height(std::max(submit->fixed_height(), entry->fixed_height()));

	m_parent->perform_layout(screen->nvg_context());

	save_instance(*this);
	set_write_line_callback([this] (const char* str) {
		console_out->append_line(str);
	});

	// Open the standard terminal library (defined above)
	luaopen_term(lua());
	lua_setglobal(lua(), "term");

	// Add these standard functions to global (though they are also included in term_lib)
	add_function("clear", Builtin::clear);
	add_function("exit", Builtin::exit);


	for (auto& f : global_setup) {
		f(*this);
	}
	run_resume();

	console_out->append_line(LUA_COPYRIGHT);

}

Console::~Console() {
	if (lua_runtime.joinable()) {
		if (active())
			stop();
		lua_runtime.join();
	}
}
