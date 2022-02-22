#include <widgets/layouts/simple_row.hpp>

#include <nanogui/nanogui.h>

gui::SimpleRowLayout::SimpleRowLayout(int margin, int spacing, VerticalAnchor anchor) :
	m_margin(margin),
	m_spacing(spacing),
	m_anchor(anchor) {}

nanogui::Vector2i gui::SimpleRowLayout::preferred_size(NVGcontext* ctx, const nanogui::Widget* container_widget) const {
	int height = 0;
	int width = m_margin * 2 + m_spacing * (container_widget->children().size());
	for (nanogui::Widget* w : container_widget->children()) {
		
		nanogui::Vector2i ps = w->preferred_size(ctx);
		height = std::max(height, (w->fixed_height() > 0) ? w->fixed_height() : ps.y());

		width += w->fixed_width() > 0 ? w->fixed_width() : ps.x();

	}
	return nanogui::Vector2i(width, height);
}

void gui::SimpleRowLayout::perform_layout(NVGcontext* ctx, nanogui::Widget* container_widget) const {
	// The widget is the container: Check if it has a fixed size. If not, use its set size
	nanogui::Vector2i c_fixed_size = container_widget->fixed_size();
	nanogui::Vector2i container_size(
		c_fixed_size.x() ? c_fixed_size.x() : container_widget->width(),
		c_fixed_size.y() ? c_fixed_size.y() : container_widget->height()
	);

	int height = 0;
	int spare_width = container_size.x() - 2 * m_margin - m_spacing * (container_widget->children().size() - 1);
	int unsized_widgets = 0;

	// First pass: figure out the spare width (total size - fixed-size widgets) and container height
	for (nanogui::Widget* w : container_widget->children()) {
		if (!w->visible())
			continue;
		
		if (w->fixed_width() > 0)
			spare_width -= w->fixed_width();
		else
			++unsized_widgets;

		height = std::max(height, w->fixed_height() ? w->fixed_height() : w->preferred_size(ctx).y());
	}

	// If this container is a window, we must shift widgets down to avoid writing over the header
	// Fixing this in layouts seems like a hack, but it is how the nanogui layouts do it...
	int y_offset = 0;
	if (dynamic_cast<nanogui::Window*>(container_widget)) {
		y_offset = container_widget->theme()->m_window_header_height;
	}

	int offset = m_margin;

	// Second pass: begin placement
	for (nanogui::Widget* w : container_widget->children()) {
		if (!w->visible())
			continue;
		
		nanogui::Vector2i preferred_sz = w->preferred_size(ctx);
		nanogui::Vector2i new_pos(offset, y_offset);

		if (w->fixed_width() > 0) {
			// If the widget has a fixed width, use that width
			w->set_width(w->fixed_width());

			offset += w->fixed_width();
		} else {
			// Otherwise, ensure it has at least its preferred width
			int use_width = std::max(preferred_sz.x(), spare_width / unsized_widgets--);

			spare_width -= use_width;

			w->set_width(use_width);
			offset += use_width;
		}
		offset += m_spacing;
		
		if (m_anchor == VerticalAnchor::STRETCH) {
			w->set_height(height);
		} else if (w->fixed_height() > 0) {
			w->set_height(w->fixed_height());
		} else {
			w->set_height(preferred_sz.y());
		}

		int y_pos = 0;
		switch (m_anchor) {
			case VerticalAnchor::CENTER:
				y_pos = (height - w->height()) / 2;
				break;
			case VerticalAnchor::TOP:
				y_pos = 0;
				break;
			case VerticalAnchor::BOTTOM:
				y_pos = height - w->height();
				break;
			case VerticalAnchor::STRETCH:
				y_pos = 0;
				break;
		}
		new_pos.y() += y_pos;
		w->set_position(new_pos);

	}
	
	
}
