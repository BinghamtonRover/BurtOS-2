#pragma once

#include <mutex>
#include <string>

#include <nanogui/nanogui.h>

namespace tony::gui {

struct WindowModuleParams
{
	WindowModuleParams() = default;

	bool resizable = true;
	bool fullscreen = false;
	int colorBits = 8;
	int alphaBits = 8;
	int depthBits = 24;
	int stencilBits = 8;
	int nSamples = 0;
	unsigned int glMajor = 3;
	unsigned int glMinor = 3;
};

class WindowModule : public nanogui::Screen {
    public:

	// using loop_callback_t = std::function<void(void)>;
	// using drop_files_callback_t = std::function<bool (const std::vector<std::string>&)>;
	// using keyboard_callback_t = std::function<bool (int,int,int,int)>;

	// using Ptr = std::shared_prt<WindowModule>;
	// using ConstPtr = std::shared_ptr<const ModuleWindow>;

    WindowModule(
		const std::string& caption = std::string(), unsigned int width = 400,
		unsigned int height = 300,
		const WindowModuleParams& p = WindowModuleParams());

    virtual ~WindowModule() override;

	// template <typename... Args>
	// static Ptr Create(Args&&... args) {
	// 	return std::make_shared<WindowModule>(std::forward<Args>(args)...);
	// }


    nanogui::Window* createManagedSubWindow(const std::string& title);

	nanogui::Window* getSubWindowsUI() { return m_subWindows.ui; }

	const nanogui::Window* getSubWindowsUI() const { return m_subWindows.ui; }

	const nanogui::Window* getSubwindow(size_t index) const {
		return m_subWindows.windows.at(index);
	}

	size_t getSubwindowCount() const { return m_subWindows.windows.size(); }
	void subwindowMinimize(size_t index) { m_subWindows.minimize(index); }
	void subwindowRestore(size_t index) { m_subWindows.restore(index); }
	void subwindowSetFocused(size_t index) { m_subWindows.setFocused(index); }

	protected:
	WindowModule(const WindowModule&) = delete;
	WindowModule& operator=(const WindowModule) = delete;

	WindowModule(WindowModule&&) = delete;
	WindowModule& operator=(WindowModule&&) = delete;

	//virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override;
	//virtual void draw_contents() override;

	//virtual bool mouse_motion_event(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers) override;
	//virtual bool mouse_button_event(const nanogui::Vector2i& p, int button, bool down, int modifiers) override;
	//virtual bool scroll_event(const nanogui::Vector2i& p, const nanogui::Vector2f& rel) override;
	//virtual bool drop_event(const std::vector<std::string>& filenames) override;

	void createControlUI();

    class SubWindow : public nanogui::Window {

        WindowModule& m_parentGui;
	    const int m_subWindowIndex = 0;

	    public:
		    SubWindow(
                WindowModule& parentGui, int myIndex, Widget* parent,
                const std::string& title)
                : nanogui::Window(parent, title),
                m_parentGui(parentGui),
                m_subWindowIndex(myIndex)
		    {
		    }

            //Handle a focus change event (default implementation: record the
            //focus status, but do nothing)
            bool focus_event(bool focused) override {
                if (focused)
                    m_parentGui.m_subWindows.onSubWindowFocused(m_subWindowIndex);
                return nanogui::Window::focus_event(focused);
            }
	};

    struct SubWindows {
        SubWindows(WindowModule& p) : parent(p) {}
        std::vector<SubWindow*> windows;
        nanogui::Window* ui = nullptr;
        nanogui::ComboBox* uiCombo = nullptr;

        void minimize(int index);
		void restore(int index);
		void setFocused(int index);

		void onSubWindowFocused(int index);

		WindowModule& parent;
    };
    SubWindows m_subWindows {*this};


};


}; //namespace