#pragma once

#include <nanogui/layout.h>

namespace gui {

/*
	The simple row layout divides space between all added widgets
	Widgets with a fixed size still assume their fixed size
*/
class SimpleRowLayout : public nanogui::Layout {
	public:
		enum class VerticalAnchor {
			CENTER,
			TOP,
			BOTTOM,
			STRETCH
		};

		// Margin: space left and right of the container
		// Spacing: space between contents
		// Anchor: how to position widgets that are shorter than the container height
		SimpleRowLayout(int margin = 0, int spacing = 6, VerticalAnchor anchor = VerticalAnchor::CENTER);
	
		virtual nanogui::Vector2i preferred_size(NVGcontext* ctx, const nanogui::Widget* widget) const override;
		virtual void perform_layout(NVGcontext* ctx, nanogui::Widget* widget) const override;

		inline int margin() const { return m_margin; }
		inline int spacing() const { return m_spacing; }
		inline VerticalAnchor anchor() const { return m_anchor; }
		inline void set_margin(int px) { m_margin = px; }
		inline void set_spacing(int px) { m_spacing = px; }
		inline void set_anchor(VerticalAnchor anchor) { m_anchor = anchor; }

	protected:
		int m_margin;
		int m_spacing;
		VerticalAnchor m_anchor;

};

}