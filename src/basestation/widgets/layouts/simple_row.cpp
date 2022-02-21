#include <widgets/layouts/simple_row.hpp>

#include <nanogui/nanogui.h>

gui::SimpleRowLayout::SimpleRowLayout(int margin, int spacing) :
	m_margin(margin),
	m_spacing(spacing) {}

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

	int spare_width = container_size.x() - 2 * m_margin - m_spacing * (container_widget->children().size() - 1);
	int unsized_widgets = 0;

	// First pass: figure out the spare width (total size - fixed-size widgets)
	for (nanogui::Widget* w : container_widget->children()) {
		if (!w->visible())
			continue;
		
		if (w->fixed_width() > 0)
			spare_width -= w->fixed_width();
		else
			++unsized_widgets;

	}

	int offset = m_margin;

	// Second pass: begin placement
	for (nanogui::Widget* w : container_widget->children()) {
		if (!w->visible())
			continue;
		
		nanogui::Vector2i preferred_sz = w->preferred_size(ctx);

		w->set_position(nanogui::Vector2i(offset, 0));
		
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
							
		if (w->fixed_height() > 0) {
			w->set_height(w->fixed_height());
		} else {
			w->set_height(preferred_sz.y());
		}
	}
	
	
}
