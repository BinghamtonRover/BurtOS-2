#include "console.hpp"

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
	self.console_out->append_line("Lua stopped.");
	self.console_out->append_line("Missing implementation: Cannot close NanoGUI Window");
	self.stop();
	return 0;
}
int Console::Builtin::title(lua_State* L) {
	auto& self = rover_lua::InteractivePrompt::get_instance<Console>(L);
	const char* new_title = luaL_checkstring(L, 1);
	self.set_title(new_title);
	return 0;
}

/*
	Console Definitions
*/

void Console::compute_size() {
		// Button should already be preferred size; only resize the text entry
		int available_w = entry_bar->width() - submit->width() - layout_margin - layout_spacing;
		if (available_w < 0) available_w = 0;
		entry->set_fixed_width(available_w);
		parent()->perform_layout(screen()->nvg_context());
}

void Console::draw(NVGcontext* ctx) {
		if (!active()) {

			entry->focus_event(false);
			entry->set_editable(false);
			submit->set_enabled(false);

		}
		nanogui::Window::draw(ctx);	
}

Console::Console(nanogui::Screen* screen) : 
		nanogui::Window(screen, "Console"),
		lua_runtime(&rover_lua::InteractivePrompt::run, static_cast<rover_lua::InteractivePrompt*>(this)) {

	set_position(nanogui::Vector2i(15, 15));
	set_layout(new nanogui::GroupLayout(6));
	set_fixed_width(800);

	auto scroller = new nanogui::VScrollPanel(this);
	scroller->set_fixed_height(300);
	
	console_out = new TextArea(scroller);
	console_out->set_font("mono");

	entry_bar = new Widget(this);
	entry_bar->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Maximum, layout_margin, layout_spacing));

	entry = new FunctionBox(entry_bar, "");
	entry->set_placeholder("");
	entry->set_editable(true);
	entry->set_alignment(nanogui::TextBox::Alignment::Left);
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
	submit->set_callback([this] {
		entry->action(FunctionBox::Action::SUBMIT);
	});

	perform_layout(screen->nvg_context());

	compute_size();

	save_instance(*this);
	set_write_line_callback([this] (const char* str) {
		console_out->append_line(str);
	});

	add_function("clear", Builtin::clear);
	add_function("exit", Builtin::exit);
	add_function("title", Builtin::title);

	for (auto& f : global_setup) {
		f(*this);
	}

	console_out->append_line(LUA_COPYRIGHT);

}

Console::~Console() {
	if (lua_runtime.joinable()) lua_runtime.join();
}
