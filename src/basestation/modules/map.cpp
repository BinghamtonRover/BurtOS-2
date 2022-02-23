#include <modules/map.hpp>
#include <widgets/layouts/simple_column.hpp>
#include <nanogui/screen.h>

nanogui::Vector2i gui::Map::preferred_size(NVGcontext* ctx) const {
	return nanogui::Vector2i(
		m_fixed_size.x() ? m_fixed_size.x() : m_size.x(),
		m_fixed_size.y() ? m_fixed_size.y() : m_size.y()
	);
}

gui::Map::Map(nanogui::Widget* parent) 
    : gui::Window(parent, "Map", true) {

    set_width(500);
    set_height(500 + m_theme->m_window_header_height);

    set_layout(new gui::SimpleColumnLayout(6, 6, 6, gui::SimpleColumnLayout::HorizontalAnchor::STRETCH));

    m_map_panel = new gui::MapPanel(this);

    parent->perform_layout(screen()->nvg_context());

}