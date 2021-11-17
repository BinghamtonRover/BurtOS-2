#pragma once

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>


class Session {
	public:
		Session();
		Session(bool, int, int, int);

		void create_window(bool, int, int, int);

		static void glfw_cursor_pos_callback(GLFWwindow*, double, double);
		static void glfw_mouse_button_callback(GLFWwindow*, int, int, int);
		static void glfw_key_callback(GLFWwindow*, int, int, int, int);
		static void glfw_char_callback(GLFWwindow*, unsigned int);
		static void glfw_drop_callback(GLFWwindow*, int, const char**);
		static void glfw_scroll_callback(GLFWwindow*, double, double);
		static void glfw_framebuffer_size_callback(GLFWwindow*, int, int);

		void gui_loop();

		static Session& get_main_session();
		static void set_main_session(Session&);

		GLFWwindow& get_window();
		void set_window(GLFWwindow&);

		nanogui::Screen& get_screen();
		void set_screen(nanogui::Screen&);

		bool get_is_glfw_init();
		void set_is_glfw_init(bool);

	private:
		static Session* main_session;
		GLFWwindow* window;
		nanogui::Screen* screen;
		bool is_glfw_init;
};