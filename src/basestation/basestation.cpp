// This base station placeholder executable is configured to be built with CMake.
// The NanoGUI and GLFW dependencies can be included in basestation source files without further setup.

#include <iostream> 
#include "session.hpp"
#include "window_module.hpp"

int main() {
	std::cout << "Binghamton University Rover Team - BurtOS 2 - Base Station 2\n";
	
	//Session basestation;
	WindowModule window;
	//window.add_widget();
	window.window_loop();
	//basestation.gui_loop();
	
	return 0;
}
