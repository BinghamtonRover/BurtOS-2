#pragma once

#include <nanogui/window.h>

namespace gui {

class Module : public nanogui::Window {
	public:
		Module(nanogui::Widget* parent, const std::string& title = "Untitled", bool resizable = true, bool minimizable = true, bool closable = true);

		bool resizable() const { return m_resizable; }
		void set_resizable(bool resizable) { m_resizable = resizable; }

		bool closable() const { return m_closable; }
		bool minimizable() const { return m_minimizable; }

		virtual bool mouse_drag_event(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int mods) override;
		virtual bool mouse_motion_event(const nanogui::Vector2i&, const nanogui::Vector2i&, int, int) override;
		virtual bool mouse_button_event(const nanogui::Vector2i& pos, int button, bool down, int mods) override;

		void close();

	protected:
		enum class ResizeSide {
			NONE,
			LEFT,
			RIGHT,
			UP,
			DOWN
		};
		ResizeSide check_vertical_resize(const nanogui::Vector2i& mouse_pos);
		ResizeSide check_horizontal_resize(const nanogui::Vector2i& mouse_pos);

		constexpr static int RESIZE_ZONE_RADIUS = 15;

		ResizeSide m_resize_dir[2];
		nanogui::Vector2i m_min_size;
		bool m_resizable;
		bool m_resize;
		bool m_closable;
		bool m_minimizable;
};

}