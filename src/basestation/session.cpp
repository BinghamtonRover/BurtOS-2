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

using namespace nanogui;

Session* Session::mainSession = nullptr;

Session::Session(){
    mainSession = this;
    createWindow(true, 0, 0, 0);
}

Session::Session(bool fullscreen, int monitor, int w, int h){
    mainSession = this;
    createWindow(fullscreen, monitor, w, h);
}

void Session::createWindow(bool fullscreen, int monitor, int w, int h){

    isGLFWInit = glfwInit();

    if(!isGLFWInit){
        std::cout << "Error initializing GLFW! Exiting..." << std::endl;
        exit(-1);
    }

    glfwSetTime(0);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

    if(fullscreen){
        if(monitor < monitorCount && 0 <= monitor){
            const GLFWvidmode* mode = glfwGetVideoMode(monitors[monitor]);
            window = glfwCreateWindow(mode->width,mode->height,"window",monitors[monitor],NULL);
        }else{
            std::cout << "Monitor not valid! Exiting..." << std::endl;
            exit(-1);
        }
    }else{
        if(w != 0 && h != 0){
            window = glfwCreateWindow(w, h, "window", NULL, NULL);
        }else{
            window = glfwCreateWindow(1280, 720, "window", NULL, NULL);
        }
    }
    
    if(!window){
        glfwTerminate();
        exit(-1);
    }

    glfwSetCursorPosCallback(window, nanoCursorPosCallback); 
    glfwSetMouseButtonCallback(window, nanoMouseButtonCallback);
    glfwSetKeyCallback(window, nanoKeyCallback);
    glfwSetCharCallback(window, nanoCharCallback);
    glfwSetDropCallback(window, nanoDropCallback);
    glfwSetScrollCallback(window, nanoScrollCallback);
    glfwSetFramebufferSizeCallback(window, nanoFramebufferSizeCallback);

    glfwMakeContextCurrent(window);

#if defined(NANOGUI_GLAD)
    if (!gladLoadGLLoader(GLADloadproc) glfwGetProcAddress))
        throw std::runtime_error("Could not initialize GLAD!");
    glGetError();
#endif

    glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    screen = new Screen();
    screen->initialize(window,true);

    int width,height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0,0,width,height);
    glfwSwapInterval(0);
    glfwSwapBuffers(window);

    screen->set_visible(true);
    screen->perform_layout();
}

void Session::nanoCursorPosCallback(GLFWwindow* window, double x, double y){
    mainSession->screen->cursor_pos_callback_event(x,y);
}

void Session::nanoMouseButtonCallback(GLFWwindow* window, int button, int action, int modifiers){
    mainSession->screen->mouse_button_callback_event(button, action, modifiers);
}

void Session::nanoKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
    mainSession->screen->key_callback_event(key, scancode, action, mods);
}

void Session::nanoCharCallback(GLFWwindow* window, unsigned int codepoint){
    mainSession->screen->char_callback_event(codepoint);
}

void Session::nanoDropCallback(GLFWwindow* window, int count, const char** filenames){
    mainSession->screen->drop_callback_event(count, filenames);
}

void Session::nanoScrollCallback(GLFWwindow* window, double x, double y){
    mainSession->screen->scroll_callback_event(x,y);
}

void Session::nanoFramebufferSizeCallback(GLFWwindow* window, int width, int height){
    mainSession->screen->resize_callback_event(width, height);
}

void Session::runGUI(){
    if(isGLFWInit){
        while(!glfwWindowShouldClose(window)){
            glfwPollEvents();

            //We should probably add the window updates once we actually have
            //something to update here
        }

        glfwDestroyWindow(window);
        glfwTerminate();
    }
}
