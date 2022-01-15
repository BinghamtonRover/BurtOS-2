#pragma once

#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>
#include <iostream>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#if defined(_MSC_VER)
#  pragma warning (disable: 4505) // don't warn about dead code in stb_image.h
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include <stb_image.h>

using namespace nanogui;

class WindowModule : public Screen {
    private:
        Screen *screen = nullptr;
        FormHelper *widget;

    public:
        //will eventually change to accept parameters for type of window
        WindowModule();
        //will eventually change to accept title text
        void create_screen();
        //This function must be called after creating all widgets, groups, variables, etc.
        void window_loop();
        //Implementation of next four functions will depend on what should a window contain
        void add_widget();
        void add_group();
        void add_variable();
        void add_button();
};