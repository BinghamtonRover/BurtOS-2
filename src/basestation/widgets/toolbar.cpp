#include <widgets/toolbar.hpp>

#include <widgets/layouts/simple_row.hpp>
#include <nanogui/opengl.h>
#include <nanogui/screen.h>

gui::Toolbar::Toolbar(nanogui::Widget* parent, const nanogui::Color& bg_color) :
	nanogui::Widget(parent),
	m_background_color(bg_color) {

	//set_layout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal));
	set_layout(new gui::SimpleRowLayout(6, 0));

	// Create a left, right, and center tray with blank widgets to align them
	bool first = true;
	for (auto& tray : trays) {
		if (first)
			first = false;
		else
			new nanogui::Widget(this);

		tray = new gui::Tray(this);
	}

}

void gui::Toolbar::draw(NVGcontext* ctx) {
	nvgBeginPath(ctx);
	nvgRect(ctx, m_pos.x(), m_pos.y(), m_pos.x() + m_size.x(), m_pos.y() + m_size.y());
	nvgFillColor(ctx, m_background_color);
	nvgFill(ctx);
	nvgClosePath(ctx);

	nanogui::Widget::draw(ctx);
}

nanogui::Vector2i gui::Toolbar::preferred_size(NVGcontext* ctx) const {
	return nanogui::Vector2i(m_parent->width(), nanogui::Widget::preferred_size(ctx).y());
}

void gui::Toolbar::perform_layout(NVGcontext* ctx) {
	for (auto tray : trays) {
		tray->set_fixed_size(tray->preferred_size(ctx));
	}
	nanogui::Widget::perform_layout(ctx);

	// Place at the bottom corner of the parent
	set_position(nanogui::Vector2i(
		m_parent->position().x(),
		m_parent->position().y() + m_parent->size().y() - m_size.y()
	));
}
