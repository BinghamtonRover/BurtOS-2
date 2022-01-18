#pragma once

#include <nanogui/window.h>
#include <nanogui/textbox.h>
#include <nanogui/vscrollpanel.h>
#include "../widgets/action_textbox.hpp"
#include "../widgets/textarea.hpp"

#include <interactive_lua.hpp>

#include <iostream>
#include <thread>

class Console : protected nanogui::Window {
private:
	const int layout_margin = 0;
	const int layout_spacing = 6;
	ActionTextBox* entry;
	nanogui::Button* submit;
	nanogui::Widget* entry_bar;
	TextArea* console_out;
	std::vector<std::string> history;
	std::size_t history_index = 0;
	std::string uncommitted_entry;

	void compute_size() {
		// Button should already be preferred size; only resize the text entry
		int available_w = entry_bar->width() - submit->width() - layout_margin - layout_spacing;
		if (available_w < 0) available_w = 0;
		entry->set_fixed_width(available_w);
	}
	
	rover_lua::InteractivePrompt lua_prompt;
	std::thread lua_runtime;
public:
	Console(nanogui::Screen* screen) : 
			nanogui::Window(screen, "Console"),
			lua_runtime(&rover_lua::InteractivePrompt::run, &lua_prompt)
	{

		set_position(nanogui::Vector2i(15, 15));
		set_layout(new nanogui::GroupLayout(6));
		set_fixed_width(500);

		auto scroller = new nanogui::VScrollPanel(this);
		scroller->set_fixed_height(300);
		
		console_out = new TextArea(scroller);
		console_out->set_font("mono");

		entry_bar = new Widget(this);
		entry_bar->set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Maximum, layout_margin, layout_spacing));

		entry = new ActionTextBox(entry_bar, "");
		entry->set_placeholder("");
		entry->set_editable(true);
		entry->set_alignment(nanogui::TextBox::Alignment::Left);
		entry->set_action_callback([this] (ActionTextBox::Action a) {
			switch (a) {
				case ActionTextBox::Action::SUBMIT:
					console_out->append_line("> " + entry->value());

					if (!entry->value().empty()) {
						lua_prompt.execute_line(entry->value());
						history.push_back(entry->value());
						// Index intentionally beyond last element
						history_index = history.size();
					}

					entry->set_value("");
					break;
				case ActionTextBox::Action::SCROLL_UP:
					if (history_index > 0 && !history.empty()) {
						if (history_index == history.size()) {
							uncommitted_entry = entry->value();
						}
						history_index--;
						entry->set_value(history[history_index]);
					}
					break;
				case ActionTextBox::Action::SCROLL_DOWN:
					if (history_index + 1 < history.size()) {
						history_index++;
						entry->set_value(history[history_index]);
					} else if (history_index != history.size()) {
						history_index = history.size();
						entry->set_value(uncommitted_entry);
					}
					break;
				case ActionTextBox::Action::CANCEL: 
					if (entry->value().empty()) {
						lua_prompt.interrupt();
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
			entry->action(ActionTextBox::Action::SUBMIT);
		});

		perform_layout(screen->nvg_context());

		compute_size();

		lua_prompt.set_write_line_callback([this] (const char* str) {
			console_out->append_line(str);
		});

		console_out->append_line(LUA_COPYRIGHT);

	}

};
