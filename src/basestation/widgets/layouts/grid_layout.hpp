#pragma once

#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/formhelper.h>

class GridLayout : public nanogui::Screen {
    public:
        GridLayout(nanogui::Screen*);
        void add_window(nanogui::Window*);

    private:
        void update_grid(nanogui::Window*);

    private:
        Screen* screen = nullptr;
        nanogui::FormHelper* widget;
        std::vector<nanogui::Window*> arr_windows;
        int columns;
        int rows;
        int max_columns = 2;
        int max_rows = 3;
        int grid_width;
        int grid_height;
        int margin = 15;
};