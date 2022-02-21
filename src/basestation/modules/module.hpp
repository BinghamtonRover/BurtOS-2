#pragma once

#include <nanogui/window.h>

namespace gui {

class Module : public nanogui::Window {
	public:
		Module(nanogui::Widget* parent, const std::string& title = "Untitled", bool resizable = true);

		bool resizable() const { return m_resizable; }
		void set_resizable(bool resizable) { m_resizable = resizable; }

		virtual bool mouse_drag_event(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int mods) override;
		virtual bool mouse_motion_event(const nanogui::Vector2i&, const nanogui::Vector2i&, int, int) override;
		virtual bool mouse_button_event(const nanogui::Vector2i& pos, int button, bool down, int mods) override;

	protected:
		int check_vertical_resize(const nanogui::Vector2i& mouse_pos);
		int check_horizontal_resize(const nanogui::Vector2i& mouse_pos);

		nanogui::Vector2i m_resize_dir;
		nanogui::Vector2i m_min_size;
		bool m_resizable;
		bool m_resize;
};

}