#include "window_module.hpp"

WindowModule::WindowModule() {
    nanogui::init();
    create_screen();
}

void WindowModule::create_screen() {
    screen = new Screen(Vector2i(500, 700), "Window Module Test");
    screen->set_visible(true);
}

void WindowModule::add_widget() {
    widget = new FormHelper(screen);
    ref<Window> window = widget->add_window(Vector2i(10, 10), "I am a widget");
    widget->add_group("I am a group inside a widget");
    widget->add_button("I am a button", []() { std::cout << "Button pressed." << std::endl; });
    window->center();
}

void WindowModule::add_group() {
    
}

void WindowModule::add_button() {
    
}

void WindowModule::window_loop() {
    //window stays active until exited out
    mainloop(-1);
    shutdown();
}