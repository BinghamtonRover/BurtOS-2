#include "controller_binding_popup.hpp"

BindingPopup::BindingPopup(nanogui::Screen* screen, ControllerManager& c, int js_id, int ax_idx)
		: nanogui::Window(screen, "Edit Binding"),
		sel_joystick_id(js_id),
		sel_axis_idx(ax_idx),
		ctrl_manager(c) {
	
	set_layout(new nanogui::GroupLayout());
	set_fixed_width(200);

	std::vector<std::string> action_names( {"Nothing"});
	for (const auto& action : ctrl_manager.actions()) {
		action_names.push_back(action.name());
	}

	bind_selector = new nanogui::ComboBox(this, action_names);
	bind_selector->set_callback([this](int i) {
		try {
			JoystickAxis& target = ctrl_manager.devices().at(sel_joystick_id).axes().at(sel_axis_idx);
			if (i == 0) {
				target.unbind();
			} else {
				// action names includes "nothing", which isn't an actual action, so index is off by 1
				i--;
				target.set_action(ctrl_manager.actions().at(i));
			}
		} catch (const std::out_of_range&) {

		}
	});

	auto close_button = new nanogui::Button(this, "Close", FA_WINDOW_CLOSE);
	close_button->set_callback([this]() {
		set_visible(false);
	});

}

void BindingPopup::draw(NVGcontext* ctx) {
	// If there is a selection, but the selection is not present or out of range, deselect and hide window
	if (sel_joystick_id >= 0) {
		try {
			if (!ctrl_manager.devices().at(sel_joystick_id).present()) {
				sel_joystick_id = -1;
				set_visible(false);
			} else {
				ctrl_manager.devices().at(sel_joystick_id).axes().at(sel_axis_idx);
			}
		} catch (const std::out_of_range&) {
			sel_joystick_id = -1;
			set_visible(false);
		}
	}
	nanogui::Window::draw(ctx);
}

void BindingPopup::set_selection(int new_joystick_id, int new_axis_idx) {
	sel_joystick_id = new_joystick_id;
	sel_axis_idx = new_axis_idx;

	// Update the bind selector to reflect this axis's action
	auto& target = ctrl_manager.devices().at(sel_joystick_id).axes().at(sel_axis_idx);

	// If the action is somehow not found, default to position 0 ("Nothing")
	// This could happen if a new action was added during the pop-ups lifetime, but
	// all actions should be created before any GUI windows are even initialized
	bind_selector->set_selected_index(0);
	const auto& action_names = bind_selector->items();
	for (std::size_t i = 0; i < action_names.size(); i++) {
		if (action_names[i] == target.action().name()) {
			bind_selector->set_selected_index(i);
			break;
		}
	}
}