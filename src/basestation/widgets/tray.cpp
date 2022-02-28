#include <widgets/tray.hpp>

#include <widgets/layouts/simple_row.hpp>
#include <nanogui/opengl.h>

gui::Tray::Tray(nanogui::Widget* parent) :
	nanogui::Widget(parent) {
	
	// Use a transparent style for buttons in the tray
	m_theme = new nanogui::Theme(*m_theme);
	m_theme->m_button_corner_radius = 0;

	nanogui::Color full_transparency(0, 0, 0, 0);
	nanogui::Color light_shading(1.0F, 1.0F, 1.0F, 0.2F);
	nanogui::Color heavy_shading(1.0F, 1.0F, 1.0F, 0.4F);

	m_theme->m_button_gradient_bot_unfocused = full_transparency;
	m_theme->m_button_gradient_top_unfocused = full_transparency;

	m_theme->m_button_gradient_bot_focused = light_shading;
	m_theme->m_button_gradient_top_focused = light_shading;

	m_theme->m_button_gradient_bot_pushed = heavy_shading;
	m_theme->m_button_gradient_top_pushed = heavy_shading;

	m_theme->m_border_light = full_transparency;
	m_theme->m_border_dark = full_transparency;

	set_layout(new gui::SimpleRowLayout(0, 4));
}
