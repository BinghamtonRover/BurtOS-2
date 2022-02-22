#include <widgets/layouts/simple_column.hpp>

#include <nanogui/nanogui.h>

gui::SimpleColumnLayout::SimpleColumnLayout(int horizontal_margin, int vertical_margin, int spacing, HorizontalAnchor anchor) :
	m_horizontal_margin(horizontal_margin),
	m_vertical_margin(vertical_margin),
	m_spacing(spacing),
	m_anchor(anchor) {}

nanogui::Vector2i gui::SimpleColumnLayout::preferred_size(NVGcontext* ctx, const nanogui::Widget* container_widget) const {
	int width = 0;
	int height = m_vertical_margin * 2;
	if (dynamic_cast<const nanogui::Window*>(container_widget)) {
		height += container_widget->theme()->m_window_header_height;
	}
	int visible_widgets = 0;

	for (nanogui::Widget* w : container_widget->children()) {
		if (!w->visible())
			continue;
		++visible_widgets;
		nanogui::Vector2i ps = w->preferred_size(ctx);
		width = std::max(width, (w->fixed_width() > 0) ? w->fixed_width() : ps.x());

		height += w->fixed_height() > 0 ? w->fixed_height() : ps.y();

	}
	width += m_horizontal_margin * 2;
	height += std::max((visible_widgets - 1) * m_spacing, 0);

	return nanogui::Vector2i(width, height);
}

void gui::SimpleColumnLayout::perform_layout(NVGcontext* ctx, nanogui::Widget* container_widget) const {
	// The widget is the container: Check if it has a fixed size. If not, use its set size
	nanogui::Vector2i c_fixed_size = container_widget->fixed_size();
	nanogui::Vector2i container_size(
		c_fixed_size.x() ? c_fixed_size.x() : container_widget->width(),
		c_fixed_size.y() ? c_fixed_size.y() : container_widget->height()
	);


	// If this container is a window, we must shift widgets down to avoid writing over the header
	// Fixing this in layouts seems like a hack, but it is how the nanogui layouts do it...
	int y_offset = m_vertical_margin;
	if (dynamic_cast<nanogui::Window*>(container_widget)) {
		y_offset += container_widget->theme()->m_window_header_height;
	}

	int width = 0;
	// All usable space: container size less the top offset (y_offset), the bottom margin (not included in y_offset), and spacing
	int spare_height = container_size.y() - y_offset - m_vertical_margin;
	int visible_children = 0;
	int unsized_widgets = 0;

	// First pass: figure out the spare height (total size - fixed-size widgets) and the desired container width
	for (nanogui::Widget* w : container_widget->children()) {
		if (!w->visible())
			continue;
		++visible_children;
		
		if (w->fixed_height() > 0)
			spare_height -= w->fixed_height();
		else
			++unsized_widgets;

		width = std::max(width, w->fixed_width() ? w->fixed_width() : w->preferred_size(ctx).x());
	}
	spare_height -= m_spacing * (visible_children - 1);
	spare_height = std::max(spare_height, 0);

	// Limit the full width to between the container margins
	width = std::min(width, container_size.x() - m_horizontal_margin * 2);

	// Second pass: begin placement
	for (nanogui::Widget* w : container_widget->children()) {
		if (!w->visible())
			continue;
		
		nanogui::Vector2i preferred_sz = w->preferred_size(ctx);
		nanogui::Vector2i new_pos(m_horizontal_margin, y_offset);

		if (w->fixed_height() > 0) {
			// If the widget has a fixed height, use that height
			w->set_height(w->fixed_height());

			y_offset += w->fixed_height();
		} else {
			// Otherwise, try to ensure each gets its preferred height, but do not exceed the spare height
			// In some conditions, some unsized widgets may not fit in the layout
			int use_height = std::max(std::min(preferred_sz.y(), spare_height), spare_height / unsized_widgets--);

			spare_height -= use_height;

			w->set_height(use_height);
			y_offset += use_height;
		}
		y_offset += m_spacing;
		
		if (m_anchor == HorizontalAnchor::STRETCH) {
			w->set_width(container_size.x() - m_horizontal_margin * 2);
		} else if (w->fixed_width() > 0) {
			w->set_width(std::min(w->fixed_width(), width));
		} else {
			w->set_width(std::min(preferred_sz.x(), width));
		}

		int x_pos = 0;
		switch (m_anchor) {
			case HorizontalAnchor::CENTER:
				x_pos = (width - w->width()) / 2;
				break;
			case HorizontalAnchor::LEFT:
				x_pos = 0;
				break;
			case HorizontalAnchor::RIGHT:
				x_pos = width - w->width();
				break;
			case HorizontalAnchor::STRETCH:
				x_pos = 0;
				break;
		}
		new_pos.x() += x_pos;
		w->set_position(new_pos);
		w->perform_layout(ctx);

	}
	
	
}
