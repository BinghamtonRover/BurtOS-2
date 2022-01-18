#pragma once

#if defined(NANOGUI_GLAD)
	#if defined(NANOGUI_SHARED) && !defined(GLAD_GLAPI_EXPORT)
		#define GLAD_GLAPI_EXPORT
	#endif

	#include <glad/glad.h>
#else
	#if defined(__APPLE__)
		#define GLFW_INCLUDE_GLCOREARB
	#else
		#define GL_GLEXT_PROTOTYPES
	#endif
#endif

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>
#include "modules/console.hpp"


class Session {
	public:
		/*
		@param fullscreen
		@param monitor Monitor used in fullscreen mode. Set to -1 to use the system's primary monitor.
		@param w Window width in windowed mode. Not used in fullscreen mode.
		@param h Window height in windowed mode. Not used in fullscreen mode.
		*/
		Session(bool fullscreen=true, int monitor=-1, int w=1280, int h=720);

		static void glfw_cursor_pos_callback(GLFWwindow*, double, double);
		static void glfw_mouse_button_callback(GLFWwindow*, int, int, int);
		static void glfw_key_callback(GLFWwindow*, int, int, int, int);
		static void glfw_char_callback(GLFWwindow*, unsigned int);
		static void glfw_drop_callback(GLFWwindow*, int, const char**);
		static void glfw_scroll_callback(GLFWwindow*, double, double);
		static void glfw_framebuffer_size_callback(GLFWwindow*, int, int);

		// Does not return until the GUI should exit
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
		const static char* window_title;
		static Session* main_session;
		GLFWwindow* window;
		nanogui::Screen* screen;
		Console* main_console;
		bool is_glfw_init;

		void create_window(bool, int, int, int);
};
