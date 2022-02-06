#pragma once

#include <mutex>
#include <string>
#include "stacked_widget.hpp"

#include <nanogui/nanogui.h>

namespace module::gui {

struct WindowModuleParams
{
	WindowModuleParams() = default;

	bool resizable = true;
	bool fullscreen = false;
	int color_bits = 8;
	int alpha_bits = 8;
	int depth_bits = 24;
	int stencil_bits = 8;
	int n_samples = 0;
	unsigned int gl_major = 3;
	unsigned int gl_minor = 3;
};

class WindowModule : public nanogui::Screen {
    public:

    WindowModule(
		const std::string& caption = std::string(), unsigned int width = 400,
		unsigned int height = 300,
		const WindowModuleParams& p = WindowModuleParams());

    virtual ~WindowModule() override;

    nanogui::Window* create_subwindow(const std::string& title);
	nanogui::Window* get_subwindows_ui() { return m_subwindows.ui; }
	const nanogui::Window* get_subwindows_ui() const { return m_subwindows.ui; }
	const nanogui::Window* get_subwindow(size_t index) const { return m_subwindows.windows.at(index); }
	size_t get_subwindow_count() const { return m_subwindows.windows.size(); }
	void subwindow_minimize(size_t index) { m_subwindows.minimize(index); }
	void subwindow_restore(size_t index) { m_subwindows.restore(index); }
	void subwindow_set_focused(size_t index) { m_subwindows.set_focused(index); }

	protected:
	WindowModule(const WindowModule&) = delete;
	WindowModule& operator=(const WindowModule) = delete;

	WindowModule(WindowModule&&) = delete;
	WindowModule& operator=(WindowModule&&) = delete;

	void create_manager_ui();

    class SubWindow : public nanogui::Window {

        WindowModule& m_parent_gui;
	    const int m_subwindow_index = 0;

	    public:
		    SubWindow(
                WindowModule& parent_gui, int my_index, Widget* parent,
                const std::string& title)
                : nanogui::Window(parent, title),
                m_parent_gui(parent_gui),
                m_subwindow_index(my_index)
		    {
		    }

            //Handle a focus change event (default implementation: record the
            //focus status, but do nothing)
            bool focus_event(bool focused) override {
                if (focused)
                    m_parent_gui.m_subwindows.on_subwindow_focused(m_subwindow_index);
                return nanogui::Window::focus_event(focused);
            }
	};

    struct SubWindows {
        SubWindows(WindowModule& p) : parent(p) {}
        std::vector<SubWindow*> windows;
        nanogui::Window* ui = nullptr;
        nanogui::ComboBox* ui_combo = nullptr;

		//void close(int index);
        void minimize(int index);
		void restore(int index);
		void set_focused(int index);

		void on_subwindow_focused(int index);

		WindowModule& parent;
    };
    SubWindows m_subwindows {*this};


};


}; //namespace