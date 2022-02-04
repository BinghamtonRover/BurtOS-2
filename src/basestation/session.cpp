#include "session.hpp"

#include <iostream>

#include "controls/lua_ctrl_lib.hpp"
#include "modules/console.hpp"

const char* Session::window_title = "Base Station 2.0 - Binghamton University Rover Team";

Session* Session::main_session = nullptr;

Session::Session(bool fullscreen, int monitor, int w, int h) {
	main_session = this;
	create_window(fullscreen, monitor, w, h);
}

void Session::create_window(bool fullscreen, int monitor, int w, int h) {

	is_glfw_init = glfwInit();

	if (!is_glfw_init) {
		std::cerr << "Error initializing GLFW! Exiting..." << std::endl;
		exit(-1);
	}

	glfwSetTime(0);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


	GLFWmonitor* use_monitor = nullptr;
	if (fullscreen) {
		int monitor_count;
		GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
		if(monitor < monitor_count && monitor > 0){
			use_monitor = monitors[monitor];
		} else {
			use_monitor = glfwGetPrimaryMonitor();
		}     
		const GLFWvidmode* mode = glfwGetVideoMode(use_monitor);
		w = mode->width;
		h = mode->height;
	}
	window = glfwCreateWindow(w, h, window_title, use_monitor, nullptr);

	if (!window) {
		std::cerr << "Error creating main window. Exiting..." << std::endl;
		glfwTerminate();
		exit(-1);
	}

	glfwSetCursorPosCallback(window, glfw_cursor_pos_callback); 
	glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
	glfwSetKeyCallback(window, glfw_key_callback);
	glfwSetCharCallback(window, glfw_char_callback);
	glfwSetDropCallback(window, glfw_drop_callback);
	glfwSetScrollCallback(window, glfw_scroll_callback);
	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);

	glfwMakeContextCurrent(window);

#if defined(NANOGUI_GLAD)
	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		throw std::runtime_error("Could not initialize GLAD!");
	}
	glGetError();
#endif

	glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	controller_mgr.init();

	screen = new nanogui::Screen();
	screen->initialize(window,true);

	int width,height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0,0,width,height);
	glfwSwapInterval(0);
	glfwSwapBuffers(window);


	screen->set_visible(true);
	screen->perform_layout();

	Console::add_setup_routine([](Console& new_console) {
		new_console.add_function("shutdown", [](lua_State*) {
			glfwSetWindowShouldClose(main_session->window, 1);
			return 0;
		});
		new_console.load_library("ctrl", lua_ctrl_lib::open);
	});

	new Console(screen);
}

void Session::glfw_cursor_pos_callback(GLFWwindow* window, double x, double y) {
	main_session->screen->cursor_pos_callback_event(x,y);
}

void Session::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int modifiers) {
	main_session->screen->mouse_button_callback_event(button, action, modifiers);
}

void Session::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	main_session->screen->key_callback_event(key, scancode, action, mods);
}

void Session::glfw_char_callback(GLFWwindow* window, unsigned int codepoint) {
	main_session->screen->char_callback_event(codepoint);
}

void Session::glfw_drop_callback(GLFWwindow* window, int count, const char** filenames) {
	main_session->screen->drop_callback_event(count, filenames);
}

void Session::glfw_scroll_callback(GLFWwindow* window, double x, double y) {
	main_session->screen->scroll_callback_event(x,y);
}

void Session::glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	main_session->screen->resize_callback_event(width, height);
}

void Session::gui_loop() {
	if (is_glfw_init) {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			
			controller_mgr.update_controls();

			glClearColor(0.11F, 0.11F, 0.11F, 1.0F);
			glClear(GL_COLOR_BUFFER_BIT);


			screen->draw_contents();
			screen->draw_widgets();

			glfwSwapBuffers(window);

			for (const auto& event_callback : scheduled_events) {
				event_callback(*this);
			}
			scheduled_events.clear();
		}

		glfwDestroyWindow(window);
		glfwTerminate();
	}
}

Session& Session::get_main_session() {
	return (*main_session);
}

void Session::set_main_session(Session& new_sesion){
	main_session = &new_sesion;
}

GLFWwindow& Session::get_window() {
	return (*window);
}

void Session::set_window(GLFWwindow& new_window) {
	window = &new_window;
}

nanogui::Screen& Session::get_screen() {
	return (*screen);
}

void Session::set_screen(nanogui::Screen& new_screen) {
	screen = &new_screen;
}

bool Session::get_is_glfw_init() {
	return is_glfw_init;
}

void Session::set_is_glfw_init(bool is_init) {
	is_glfw_init = is_init;
}

void Session::schedule_sync_event(const std::function<void(Session&)>& call) {
	std::lock_guard lock(schedule_lock);
	scheduled_events.push_back(call);
}