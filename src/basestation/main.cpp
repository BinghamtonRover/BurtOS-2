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


		try {
			boost::property_tree::ptree settings;
			session.write_settings(settings);

			// Because we are saving after all windows are closed, carry over the launch screen settings
			settings.erase("screens");
			auto scr_cfg = basestation_settings.get_child_optional("screens");
			if (scr_cfg)
				settings.put_child("screens", scr_cfg.get());

			namespace fs = std::filesystem;
			fs::path userdata(fs::path(argv[0]).parent_path() / "userdata");
			if (!fs::exists(userdata)) {
				fs::create_directory(userdata);
			}
			if (fs::is_directory(userdata)) {

				fs::path settings_file = userdata / "basestation-settings.json";

				boost::property_tree::ptree wrapped;
				wrapped.add_child("basestation", settings);

				if (settings != basestation_settings) {
					std::cout << "Saving changes to user settings\n";
					boost::property_tree::json_parser::write_json(settings_file.string(), wrapped);
				}

			} else {
				throw std::runtime_error("userdata is not a directory");
			}

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
