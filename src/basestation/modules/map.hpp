#pragma once

#include <widgets/window.hpp>
#include <widgets/map_panel.hpp>

namespace gui {

class Map : public gui::Window {
	public:
		Map(nanogui::Widget* parent);

		inline gui::MapPanel* map_panel() {
			return m_map_panel;
		}

		virtual nanogui::Vector2i preferred_size(NVGcontext*) const override;

	private:

		gui::MapPanel* m_map_panel;


};

}