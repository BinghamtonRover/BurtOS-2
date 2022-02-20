#include <iostream>

#include <basestation.hpp>
#include <basestation_screen.hpp>

int main() {
	std::cout << "BurtOS 2 - Base Station v2\n";
	std::cout << "Copyright (C) 2022 Binghamton University Rover Team\n";

	try {
		register_messages();

		nanogui::init();

		Basestation session;
		
		session.add_screen(new BasestationScreen());
		
		session.mainloop();

		nanogui::shutdown();

	} catch (const std::exception& e) {
		std::cerr << "Fatal error: Unhandled '" << typeid(e).name() << "' exception!\n\twhat(): " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
