#include <basestation.hpp>
#include <modules/console.hpp>
#include <controls/lua_ctrl_lib.hpp>

#include <stdexcept>

#include <nanogui/opengl.h>

Basestation* Basestation::main_instance = nullptr;

Basestation::Basestation()
	: subsystem_sender(main_thread_ctx),
	remote_drive(subsystem_sender) {

	if (main_instance != nullptr) {
		throw std::runtime_error("Basestation::Basestation: duplicate instance not allowed");
	}
	main_instance = this;

	controller_mgr.init();
	remote_drive.add_controller_actions(controller_mgr);

	Console::add_setup_routine([](Console& new_console) {
		new_console.load_library("ctrl", lua_ctrl_lib::open);
		new_console.load_library("bs", lua_basestation_lib::open);
	});
}

Basestation::~Basestation() {
	main_instance = nullptr;
}

void Basestation::add_screen(BasestationScreen* new_scr) {
	screens.push_back(new_scr);
}

BasestationScreen* Basestation::get_focused_screen() const {
	if (screens.size() == 0)
		throw std::runtime_error("Basestation::get_focused_screen: no screens");
	
	// If there's no focused screen, try to use a visible screen
	BasestationScreen* vis = screens[0];
	for (auto& scr : screens) {
		if (scr->visible()) {
			if (scr->focused()) {
				return scr;
			}
			vis = scr;
		}
	}
	return vis;
}

void Basestation::mainloop() {
	while (continue_operating) {
		glfwPollEvents();
		main_thread_ctx.poll();

		remote_drive.poll_events();

		controller_mgr.update_controls();

		{
			std::lock_guard lock(schedule_lock);
			for (auto& callback : async_callbacks) {
				callback(*this);
			}
			async_callbacks.clear();
		}

		// Redraw open screens and close others
		// Closed screens will be erased from the vector:
		// On erase, recalculate end, but do not ++it
		auto it = screens.begin();
		auto end = screens.end();
		while (it != end) {
			auto scr = *it;
			if (!scr->visible()) {
				continue;
			} else if (glfwWindowShouldClose(scr->glfw_window())) {
				scr->set_visible(false);
				it = screens.erase(it);
				end = screens.end();
				continue;
			}
			scr->redraw();
			scr->draw_all();

			++it;
		}

		if (screens.size() == 0) {
			break;
		}
	}
}

void Basestation::schedule(const std::function<void(Basestation&)>& callback) {
	std::lock_guard lock(schedule_lock);
	async_callbacks.push_back(callback);
}

const struct luaL_Reg Basestation::lua_basestation_lib::lib[] = {
	{"shutdown", shutdown},
	{"new_screen", new_screen},
	{NULL, NULL}
};

int Basestation::lua_basestation_lib::shutdown(lua_State*) {
	main_instance->continue_operating = false;
	return 0;
}

int Basestation::lua_basestation_lib::new_screen(lua_State*) {
	async([](Basestation& bs) {
		bs.add_screen(new BasestationScreen());
	});
	return 0;
}

void Basestation::lua_basestation_lib::open(lua_State* L) {
	luaL_newlib(L, lib);
}
