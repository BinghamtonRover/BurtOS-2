#pragma once

#include <nanogui/widget.h>
#include <widgets/tray.hpp>
#include <array>

namespace gui {

class Toolbar : public nanogui::Widget {
	public:
		Toolbar(nanogui::Widget* parent, const nanogui::Color& bg_color = nanogui::Color(0, 122, 204, 255));

		const nanogui::Color& background_color() const { return m_background_color; }
		void set_background_color(const nanogui::Color& c) { m_background_color = c; }
		
		gui::Tray* left_tray() { return trays[0]; }
		gui::Tray* center_tray() { return trays[1]; }
		gui::Tray* right_tray() { return trays[2]; }

		virtual void draw(NVGcontext* ctx) override;
		virtual void perform_layout(NVGcontext* ctx) override;
		virtual nanogui::Vector2i preferred_size(NVGcontext* ctx) const override;

	private:
		nanogui::Color m_background_color;

		std::array<gui::Tray*, 3> trays;
};

}
