#pragma once

#include <nanogui/widget.h>
#include <utility>

namespace gui {

class MapPanel : public nanogui::Widget {

	public:
		MapPanel(nanogui::Widget* parent);

		virtual void draw(NVGcontext* ctx) override;
		void draw_rover(NVGcontext* ctx);
		void draw_basestation(NVGcontext* ctx);
		void draw_grid(NVGcontext* ctx);

		inline std::pair<double,double>& rover_position() {
			return m_rover_position;
		}
		inline std::pair<double,double>& basestation_position() {
			return m_basestation_position;
		}
		inline void set_rover_position(const std::pair<double,double>& pos) { // latitude, longitude
			m_rover_position = pos;
		}
		inline void set_basestation_position(const std::pair<double,double>& pos) { // latitude, longitude
			m_basestation_position = pos;
		}
		inline void set_feet_per_square(const double feet) {
			m_feet_per_square = feet;
		}

	private:
		std::pair<double, double> m_rover_position;
		std::pair<double, double> m_basestation_position;
		double m_feet_per_square = 500;
		double grid_dimension = 20;

		double get_bearing_to_base_in_rad();
		double get_distance_to_base_in_ft();
		double radians(double c);

};

}
