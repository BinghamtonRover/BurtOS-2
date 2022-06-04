#include "grid_layout.hpp"

GridLayout::GridLayout(nanogui::Screen* screen) {
    grid_width = screen->size().x() - margin;
    grid_height = screen->size().y() - margin;
    rows = 1;
    columns = 1;
    
    this->screen = screen;
    widget = new nanogui::FormHelper(screen);
}

void GridLayout::add_window(nanogui::Window* window) {
    if (columns == max_columns && rows == max_rows) return;
    window = widget->add_window(nanogui::Vector2i(margin, margin), window->title());
    arr_windows.push_back(window);
    update_grid(window);

    if (rows % 3 == 0) {
        columns++;
        rows = columns;
    } else {
        rows++;
    }
}

void GridLayout::update_grid(nanogui::Window* window) {
    arr_windows[0]->set_fixed_width(grid_width / columns - margin);
    arr_windows[0]->set_fixed_height(grid_height / rows - margin);
    for (int i = 1; i < arr_windows.size(); i++) {
        arr_windows[i]->set_fixed_height(grid_height/ rows - margin);
        arr_windows[i]->set_fixed_width(grid_width/ columns - margin);
        if (columns == 1) {
            arr_windows[i]->set_position(nanogui::Vector2i(margin, (arr_windows[i-1]->fixed_height() * i) + margin));
        } else {
            if ((i+1) % columns == 0) 
                arr_windows[i]->set_position(nanogui::Vector2i(margin + arr_windows[i-1]->fixed_width(), margin + ((i / rows) * arr_windows[i-1]->fixed_height())));
            else
                arr_windows[i]->set_position(nanogui::Vector2i(margin, arr_windows[i-1]->fixed_height() + margin) );
        }
    }
    screen->perform_layout();
}