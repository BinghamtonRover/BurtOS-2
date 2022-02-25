#include <widgets/map_panel.hpp>
#include <nanogui/opengl.h>
#include <iostream>
#include <math.h>

gui::MapPanel::MapPanel(nanogui::Widget* parent) :
	nanogui::Widget(parent),
	m_rover_position(40.7167, -74.0152),
	m_basestation_position(40.7218, -74.0094) {}

void gui::MapPanel::draw(NVGcontext* ctx) {
	nanogui::Widget::draw(ctx);

	nvgBeginPath(ctx);
	nvgRect(ctx, m_pos.x(), m_pos.y(), m_pos.x() + m_size.x(), m_pos.y() + m_size.y());
	nvgFillColor(ctx, nvgRGBA(0,0,0,255));
	nvgFill(ctx);
	nvgClosePath(ctx);

	// changes grid to size [-grid_dimension, grid_dimension] x [-grid_dimension, grid_dimension]
	nvgTranslate(ctx, m_pos.x() + m_size.x() / 2.0, m_pos.y() + m_size.y() / 2.0);
	nvgScale(ctx, m_size.x() / grid_dimension, m_size.y() / grid_dimension);

	// change grid to have North at the top
	nvgRotate(ctx, nvgDegToRad(-90));

	draw_grid(ctx);
	draw_rover(ctx);
	draw_basestation(ctx);
}

void gui::MapPanel::draw_rover(NVGcontext* ctx) {
	nvgBeginPath(ctx);

	// rover is always at center of map
	nvgMoveTo(ctx, 0.25, 0);
	nvgLineTo(ctx, -0.25, 0.25);
	nvgLineTo(ctx, -0.25, -0.25);
	nvgLineTo(ctx, 0.25, 0);
	nvgFillColor(ctx, nvgRGBA(255,0,0,255));
	nvgFill(ctx);
	nvgClosePath(ctx);
}

void gui::MapPanel::draw_basestation(NVGcontext* ctx) {
	nvgBeginPath(ctx);

	double x = get_distance_to_base_in_ft() * cos(get_bearing_to_base_in_rad()) / m_feet_per_square;
	double y = get_distance_to_base_in_ft() * sin(get_bearing_to_base_in_rad()) / m_feet_per_square;

	nvgRect(ctx, x, y, 0.5, 0.5);
	nvgFillColor(ctx, nvgRGBA(0,0,255,255));
	nvgFill(ctx);
	nvgClosePath(ctx);
}

void gui::MapPanel::draw_grid(NVGcontext* ctx) {
	nvgBeginPath(ctx);
	nvgStrokeColor(ctx, nvgRGBA(0,255,0,255));
	nvgStrokeWidth(ctx, 0.01F);
	for (int x = -grid_dimension/2; x <= grid_dimension/2; x++) {
		nvgMoveTo(ctx, x, -grid_dimension);
		nvgLineTo(ctx, x, grid_dimension);
	}
	for (int y = -grid_dimension/2; y <= grid_dimension/2; y++) {
		nvgMoveTo(ctx, -grid_dimension, y);
		nvgLineTo(ctx, grid_dimension, y);
	}
	nvgStroke(ctx);
	nvgClosePath(ctx);
}

// formula taken from https://gis.stackexchange.com/questions/252672/calculate-bearing-between-two-decimal-gps-coordinates-arduino-c
double gui::MapPanel::get_bearing_to_base_in_rad() {
	double rover_lat = m_rover_position.first;
	double rover_long = m_rover_position.second;
	double basestation_lat = m_basestation_position.first;
	double basestation_long = m_basestation_position.second;

	double teta1 = nvgDegToRad(rover_lat);
	double teta2 = nvgDegToRad(basestation_lat);
	double delta1 = nvgDegToRad(basestation_lat - rover_lat);
	double delta2 = nvgDegToRad(basestation_long - rover_long);

	double y = sin(delta2) * cos(teta2);
   	double x = cos(teta1)*sin(teta2) - sin(teta1)*cos(teta2)*cos(delta2);
	double bearing = atan2(y,x);

	return bearing;
}

// formula taken from https://www.geeksforgeeks.org/haversine-formula-to-find-distance-between-two-points-on-a-sphere/
double gui::MapPanel::get_distance_to_base_in_ft() {
	double rover_lat = m_rover_position.first;
	double rover_long = m_rover_position.second;
	double basestation_lat = m_basestation_position.first;
	double basestation_long = m_basestation_position.second;

	double dLat = nvgDegToRad(basestation_lat - rover_lat);
	double dLong = nvgDegToRad(basestation_long - rover_long);
	rover_lat = nvgDegToRad(rover_lat);
	basestation_lat = nvgDegToRad(basestation_lat);

	double distance = pow(sin(dLat / 2), 2) + pow(sin(dLong / 2), 2) * cos(rover_lat) * cos(basestation_lat);

	double earth_radius_in_miles = 3963;
	double earth_radius_in_feet = earth_radius_in_miles * 5280;
	double c = 2 * asin(sqrt(distance));
	return earth_radius_in_feet * c;
}
