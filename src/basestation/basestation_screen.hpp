#pragma once

#include <nanogui/opengl.h>
#include <nanogui/screen.h>

/*
	Properties used for positioning and sizing the GLFW window on a monitor
*/
struct ScreenPositioning {
	constexpr static int DEFAULT_WIDTH = 1280;
	constexpr static int DEFAULT_HEIGHT = 720;

	nanogui::Vector2i size;
	nanogui::Vector2i window_pos;
	int monitor;
	bool use_fullscreen;
	
	ScreenPositioning(
		const nanogui::Vector2i& size = nanogui::Vector2i(DEFAULT_WIDTH, DEFAULT_HEIGHT),
		const nanogui::Vector2i& window_pos = 100,
		int monitor = -1,
		bool use_fullscreen = false
	);
};

/*
	GUI screen class for the base station
	
	Any number of screens may be created.
*/
class BasestationScreen : public nanogui::Screen {
	public:
		BasestationScreen(const ScreenPositioning& position_cfg = ScreenPositioning());

		// Make this window fullscreen by monitor number. If the number is out of range, use the primary monitor
		void set_fullscreen(int monitor = -1);

		// Make this window fullscreen on the provided monitor. If monitor is null, the primary monitor is used
		void set_fullscreen(GLFWmonitor* monitor);

		void set_windowed(int w = -1, int h = -1);

		inline void set_windowed(nanogui::Vector2i size) {
			set_windowed(size.x(), size.y());
		}

		inline bool windowed() const { return !m_fullscreen; }

		void move_here(nanogui::Window*);

	private:
		ScreenPositioning position;
		int preferred_monitor = -1;

		virtual bool keyboard_event(int key, int scancode, int action, int mods) override;
		virtual bool resize_event(const nanogui::Vector2i& size) override;
};
