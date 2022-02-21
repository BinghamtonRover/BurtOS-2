#pragma once

#include <nanogui/layout.h>

namespace gui {

/*
	The simple row layout divdes space between all added widgets
	Widgets with a fixed size still assume their fixed size
*/
class SimpleRowLayout : public nanogui::Layout {
	public:
		SimpleRowLayout(int margin = 0, int spacing = 6);
	
		virtual nanogui::Vector2i preferred_size(NVGcontext* ctx, const nanogui::Widget* widget) const override;
		virtual void perform_layout(NVGcontext* ctx, nanogui::Widget* widget) const override;

	protected:
		int m_margin;
		int m_spacing;
};

}