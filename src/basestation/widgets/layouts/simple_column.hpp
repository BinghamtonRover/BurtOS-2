#pragma once

#include <nanogui/layout.h>

namespace gui {

/*
	The simple column layout divides space between all added widgets
	Widgets with a fixed size still assume their fixed size
	This is slightly more accurate than the simple row layout so it can work as a "top-level" layout
*/
class SimpleColumnLayout : public nanogui::Layout {
	public:
		enum class HorizontalAnchor {
			CENTER,
			LEFT,
			RIGHT,
			STRETCH
		};

		// Margin: space around the container
		// Spacing: space between contents
		// Anchor: how to position widgets that are thinner than the container width
		SimpleColumnLayout(int h_margin = 6, int v_margin = 6, int spacing = 6, HorizontalAnchor anchor = HorizontalAnchor::CENTER);
	
		virtual nanogui::Vector2i preferred_size(NVGcontext* ctx, const nanogui::Widget* widget) const override;
		virtual void perform_layout(NVGcontext* ctx, nanogui::Widget* widget) const override;

		inline int horizontal_margin() const { return m_horizontal_margin; }
		inline int vertical_margin() const { return m_vertical_margin; }
		inline int spacing() const { return m_spacing; }
		inline HorizontalAnchor anchor() const { return m_anchor; }
		inline void set_horizontal_margin(int px) { m_horizontal_margin = px; }
		inline void set_vertical_margin(int px) { m_vertical_margin = px; }
		inline void set_spacing(int px) { m_spacing = px; }
		inline void set_anchor(HorizontalAnchor anchor) { m_anchor = anchor; }

	protected:
		int m_horizontal_margin;
		int m_vertical_margin;
		int m_spacing;
		HorizontalAnchor m_anchor;

};

}
