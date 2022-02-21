#pragma once

#include <nanogui/nanogui.h>

#include <nanogui/layout.h>

namespace gui {

class SimpleRowLayout : public nanogui::Layout {
	public:
		//SimpleRowLayout();
	
		virtual nanogui::Vector2i preferred_size(NVGcontext* ctx, const nanogui::Widget* widget) const override {

		}
		virtual void perform_layout(NVGcontext* ctx, nanogui::Widget* widget) const override {
			// The widget is the container: Check if it has a fixed size. If not, use its set size
			nanogui::Vector2i w_fixed_size = widget->fixed_size();
			nanogui::Vector2i container_size(
				w_fixed_size.x() ? w_fixed_size.x() : widget->width(),
				w_fixed_size.y() ? w_fixed_size.y() : widget->height()
			);

			int spare_width = container_size.x();
			int unsized_widgets = 0;

			// First pass: figure out the spare width (total size - fixed-size widgets)
			for (nanogui::Widget* w : widget->children()) {
				if (!w->visible())
					continue;
				
				if (w->fixed_width() > 0)
					spare_width -= w->fixed_width();
				else
					++unsized_widgets;

			}

			// Second pass: begin placement
			for (nanogui::Widget* w : widget->children()) {
				
			}
			
			
		}
};

}