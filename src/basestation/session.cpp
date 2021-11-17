#include <iostream>
#include "session.hpp"

#if defined(NANOGUI_GLAD)
    #if defined(NANOGUI_SHARED) && !defined(GLAD_GLAPI_EXPORT)
        #define GLAD_GLAPI_EXPORT
    #endif

    #include <glad/glad.h>
#else
    #if defined(__APPLE__)
        #define GLFW_INCLUDE_GLCOREARB
    #else
        #define GL_GLEXT_PROTOTYPES
    #endif
#endif

Session* Session::main_session = nullptr;

Session::Session() {
    main_session = this;
    create_window(true, 0, 0, 0);
}

Session::Session(bool fullscreen, int monitor, int w, int h) {
    main_session = this;
    create_window(fullscreen, monitor, w, h);
}

void Session::create_window(bool fullscreen, int monitor, int w, int h) {

    is_glfw_init = glfwInit();

    if (!is_glfw_init) {
        std::cout << "Error initializing GLFW! Exiting..." << std::endl;
        exit(-1);
    }

    glfwSetTime(0);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    int monitor_count;
    GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);

    if (fullscreen) {
        if(monitor < monitor_count && 0 <= monitor){
            const GLFWvidmode* mode = glfwGetVideoMode(monitors[monitor]);
            window = glfwCreateWindow(mode->width,mode->height,"window",monitors[monitor],NULL);
        } else {
            std::cout << "Monitor not valid! Exiting..." << std::endl;
            exit(-1);
        }
    } else {
        if (w != 0 && h != 0) {
            window = glfwCreateWindow(w, h, "window", nullptr, nullptr);
        } else {
            window = glfwCreateWindow(1280, 720, "window", nullptr, nullptr);
        }
    }
    
    if (!window) {
        glfwTerminate();
        exit(-1);
    }

    glfwSetCursorPosCallback(window, glfw_cursor_pos_callback); 
    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetCharCallback(window, glfw_char_callback);
    glfwSetDropCallback(window, glfw_drop_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);
    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);

    glfwMakeContextCurrent(window);

#if defined(NANOGUI_GLAD)
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        throw std::runtime_error("Could not initialize GLAD!");
	}
    glGetError();
#endif

    glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    screen = new nanogui::Screen();
    screen->initialize(window,true);

    int width,height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0,0,width,height);
    glfwSwapInterval(0);
    glfwSwapBuffers(window);

    screen->set_visible(true);
    screen->perform_layout();
}

void Session::glfw_cursor_pos_callback(GLFWwindow* window, double x, double y) {
    main_session->screen->cursor_pos_callback_event(x,y);
}

void Session::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int modifiers) {
    main_session->screen->mouse_button_callback_event(button, action, modifiers);
}

void Session::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    main_session->screen->key_callback_event(key, scancode, action, mods);
}

void Session::glfw_char_callback(GLFWwindow* window, unsigned int codepoint) {
    main_session->screen->char_callback_event(codepoint);
}

void Session::glfw_drop_callback(GLFWwindow* window, int count, const char** filenames) {
    main_session->screen->drop_callback_event(count, filenames);
}

void Session::glfw_scroll_callback(GLFWwindow* window, double x, double y) {
    main_session->screen->scroll_callback_event(x,y);
}

void Session::glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    main_session->screen->resize_callback_event(width, height);
}

void Session::gui_loop() {
    if (is_glfw_init) {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            /*
            We should probably add the window updates once we actually have
            something to update here
            */
        }

        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

Session& Session::get_main_session() {
    return (*main_session);
}

void Session::set_main_session(Session& new_sesion){
    main_session = &new_sesion;
}

GLFWwindow& Session::get_window() {
	return (*window);
}

void Session::set_window(GLFWwindow& new_window) {
	window = &new_window;
}

nanogui::Screen& Session::get_screen() {
	return (*screen);
}

void Session::set_screen(nanogui::Screen& new_screen) {
	screen = &new_screen;
}

bool Session::get_is_glfw_init() {
	return is_glfw_init;
}

void Session::set_is_glfw_init(bool is_init) {
	is_glfw_init = is_init;
}