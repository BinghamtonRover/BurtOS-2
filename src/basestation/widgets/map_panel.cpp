#include <widgets/map_panel.hpp>
#include <nanogui/opengl.h>

gui::MapPanel::MapPanel(nanogui::Widget* parent) :
	nanogui::Widget(parent),
	m_rover_position(0.0, 0.0),
	m_basestation_position(0.0, 0.0) {}

void gui::MapPanel::draw(NVGcontext* ctx) {
	nanogui::Widget::draw(ctx);

	nvgBeginPath(ctx);
	nvgRect(ctx, m_pos.x(), m_pos.y(), m_pos.x() + m_size.x(), m_pos.y() + m_size.y());
	nvgFillColor(ctx, nvgRGBA(0,0,0,255));
	nvgFill(ctx);
}
