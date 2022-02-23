#pragma once

#include <nanogui/widget.h>
#include <utility>

namespace gui {

class MapPanel : public nanogui::Widget {

	public:
		MapPanel(nanogui::Widget* parent);

		virtual void draw(NVGcontext* ctx) override;

		inline std::pair<double,double>& rover_position() {
			return m_rover_position;
		}
		inline std::pair<double,double>& basestation_position() {
			return m_basestation_position;
		}
		inline void set_rover_position(const std::pair<double,double>& pos) {
			m_rover_position = pos;
		}
		inline void set_basestation_position(const std::pair<double,double>& pos) {
			m_basestation_position = pos;
		}

	private:
		std::pair<double, double> m_rover_position;
		std::pair<double, double> m_basestation_position;
	

};

}