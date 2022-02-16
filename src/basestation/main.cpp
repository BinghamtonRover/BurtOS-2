#include <iostream>
#include <filesystem>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <basestation.hpp>
#include <basestation_screen.hpp>

int main(int argc, char* argv[]) {
	std::cout << "BurtOS 2 - Base Station v2\n";
	std::cout << "Copyright (C) 2022 Binghamton University Rover Team\n";

	boost::property_tree::ptree basestation_settings;

	// Load user settings using relative paths from the executable:
	//	{parent}/basestation.exe -> {parent}/userdata/basestation-settings.json
	if (argc >= 1) {
		try {
			namespace fs = std::filesystem;
			fs::path executable_path(argv[0]);
			fs::path settings_file = executable_path.parent_path() / "userdata" / "basestation-settings.json";
			if (fs::is_regular_file(settings_file)) {
				boost::property_tree::ptree config_tree;
				boost::property_tree::json_parser::read_json(settings_file.string(), config_tree);
				auto bs_tree = config_tree.get_child_optional("basestation");
				if (bs_tree) {
					basestation_settings = bs_tree.get();
					std::cout << "Loaded user settings from " << settings_file << "\n";
				} else {
					std::cerr << "Settings file does not contain the root of a basestation settings tree.\n";
				}
			} else {
				std::cerr << "A user settings file was not found. Base station loading with defaults.\n";
			}
		} catch (const boost::property_tree::json_parser_error& err) {
			std::cerr << "Error in user settings file: " << err.message() << " (" << err.filename() << ":" << err.line() << "\n";
		} catch (const std::exception& err) {
			std::cerr << "Error while loading user settings (" << err.what() << ")\n";
		}
	} else {
		std::cerr << "Executable path unavailable; Not loading user settings.\n";
	}

	try {
		register_messages();

		nanogui::init();

		Basestation session(basestation_settings);
		
		session.mainloop();

		std::cout << "Saving user settings\n";
		try {
			boost::property_tree::ptree settings;
			session.write_settings(settings);

			namespace fs = std::filesystem;
			fs::path executable_path(argv[0]);
			fs::path settings_file = executable_path.parent_path() / "userdata" / "basestation-settings.json";

			boost::property_tree::ptree wrapped;
			wrapped.add_child("basestation", settings);

			boost::property_tree::json_parser::write_json(settings_file.string(), wrapped);

		} catch (const std::exception& e) {
			std::cerr << "Unable to save user settings: " << e.what() << "\n";
		}

		nanogui::shutdown();

	} catch (const std::exception& e) {
		std::cerr << "Fatal error: Unhandled '" << typeid(e).name() << "' exception!\n\twhat(): " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
