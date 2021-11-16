#ifndef SESSION_H
#define SESSION_H

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>


class Session{
	public:
		static Session* mainSession;
		GLFWwindow* window;
		nanogui::Screen* screen;
		bool isGLFWInit;

		Session();
		Session(bool, int, int, int);
		void createWindow(bool, int, int, int);
		static void nanoCursorPosCallback(GLFWwindow*, double, double);
		static void nanoMouseButtonCallback(GLFWwindow*, int, int, int);
		static void nanoKeyCallback(GLFWwindow*, int, int, int, int);
		static void nanoCharCallback(GLFWwindow*, unsigned int);
		static void nanoDropCallback(GLFWwindow*, int, const char**);
		static void nanoScrollCallback(GLFWwindow*, double, double);
		static void nanoFramebufferSizeCallback(GLFWwindow*, int, int);
		void runGUI();
};

#endif