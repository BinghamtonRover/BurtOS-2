#include <iostream>
#include <filesystem>

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <basestation.hpp>
#include <basestation_screen.hpp>

/*
	Session Pre-Init Variables

	These variables are used to initialize and shutdown the session,
	but aren't directly used to run the base station

	Variables the base station relies on should not be defined here
*/

bool window_fullscreen = false;
bool save_settings = true;
bool use_default_settings = false;

int w = 0;
int h = 0;

std::string settings_path_str;
std::string userdata_path_str;

std::filesystem::path settings_file;

std::vector< std::pair<std::string, std::string> > override_settings;


// Parse command line options and set the global variables for this session
// Returns false if the launch should be cancelled
static bool read_command_line_options(int argc, char* argv[]) {
	try {

		// Boost.Program_options tutorial:
		// <https://www.boost.org/doc/libs/1_63_0/doc/html/program_options/tutorial.html>
		namespace opt = boost::program_options;

		// This is the specification for options the base station supports
		opt::options_description opts("Usage");
		opts.add_options()
			("help,h", "list these options")
			("no-save", "do not save settings upon exit")
			("no-settings", "do not read from a settings file")
			("settings,s", opt::value<std::string>(&settings_path_str), "specify a file to read settings from")
			("userdata,u", opt::value<std::string>(&userdata_path_str), "specify the userdata directory")
			("fullscreen,f", "open an initial window in fullscreen mode")
			("width,w", opt::value<int>(&w)->default_value(1280), "specify window width (windowed mode only)")
			("height,h", opt::value<int>(&h)->default_value(720), "specify window height (windowed mode only)")
			("set,D", opt::value< std::vector<std::string> >(), "specify settings override values with path=value pairs")
		;

		// Positional arguments ("unnamed arguments") can be used to set settings manually
		// Example: 'basestation network.subsystem_feed.port=22201'
		opt::positional_options_description pos;
		pos.add("set", -1);

		opt::variables_map map;
		opt::store(opt::command_line_parser(argc, argv).options(opts).positional(pos).run(), map);
		opt::notify(map);

		if (map.count("help")) {
			std::cout << opts << "\n";
			return false;
		}

		// Aggregate the "flag" options that don't set the value automatically
		save_settings = map.count("no-save") == 0;
		use_default_settings = map.count("no-settings") > 0;
		window_fullscreen = map.count("fullscreen") > 0;

		// These are path=value pairs to parse out and place in the property tree
		if (map.count("set") > 0) {
			auto& positional_settings = map["set"].as<std::vector<std::string>>();
			for (const auto& setting : positional_settings) {
				std::size_t split_at = setting.find('=');
				if (split_at == std::string::npos) {
					std::cerr << "Setting override '" << setting << "' was ignored because it was given no value.\n";
				}
				// Emplace the separated path and value into the override list
				override_settings.emplace_back(setting.substr(0, split_at), setting.substr(split_at + 1));
			}
		}

	} catch (const std::exception& e) {
		std::cerr << "Invalid options specified: " << e.what() << "\n";
		return false;
	}
	return true;
}

// Load base station settings into the property tree
// Uses the userdata_path from the command line if it is specified. Otherwise, search relative to
// the executable:
//	../basestation.exe -> ../userdata/basestation-settings.json
static void load_settings(boost::property_tree::ptree& ld_to, const char* exe_path = nullptr) {
	namespace fs = std::filesystem;

	try {

		if (!settings_path_str.empty()) {
			settings_file = fs::path(settings_path_str);
		} else if (!userdata_path_str.empty()) {
			settings_file = fs::path(userdata_path_str) / "basestation-settings.json";
		} else if (exe_path) {
			fs::path userdata = fs::path(exe_path).parent_path() / "userdata";
			userdata_path_str = userdata.string();
			settings_file = userdata / "basestation-settings.json";
		} else {
			std::cerr << "No valid paths to search for settings file.\n";

			save_settings = false;
			return;
		}
		
		if (fs::is_regular_file(settings_file)) {
			boost::property_tree::json_parser::read_json(settings_file.string(), ld_to);
			std::cout << "Loaded user settings from " << settings_file << "\n";
			return;
		} else {
			std::cerr << "A user settings file was not found. Base station loading with defaults.\n";
			return;
		}
	} catch (const boost::property_tree::json_parser_error& err) {
		std::cerr << "Error in user settings file: " << err.message() << " (" << err.filename() << ":" << err.line() << ")\n";
	} catch (const std::exception& err) {
		std::cerr << "Error while loading user settings: " << err.what() << "\n";
	}

	// If there was an error, flow falls through to here
	// Do not try to overwrite an erroneous file as the user may wish to correct the problem
	save_settings = false;
	std::cerr << "Autosave is disabled due to problems with the file.\n";
}

static void write_settings(const boost::property_tree::ptree& save_from) {
	namespace fs = std::filesystem;
	try {
		
		if (!settings_file.empty()) {

			if (settings_file.has_parent_path() && !fs::exists(settings_file.parent_path())) {
				fs::create_directories(settings_file.parent_path());
			}

			boost::property_tree::json_parser::write_json(settings_file.string(), save_from);
			std::cout << "Wrote settings to " << settings_file << "\n";
		}

	} catch (const std::exception& err) {
		std::cerr << "Error saving user settings: " << err.what() << "\n";
	}
}

int main(int argc, char* argv[]) {
	std::cout << "BurtOS 2 - Base Station v2\n";
	std::cout << "Copyright (C) 2022 Binghamton University Rover Team\n";

	if (!read_command_line_options(argc, argv)) {
		std::cerr << "Launch cancelled!\n";
		return 1;
	}

	boost::property_tree::ptree user_settings;

	if (!use_default_settings) {
		// Load settings can find userdata relative to the executable path (argv[0])
		load_settings(user_settings, argc > 0 ? argv[0] : nullptr);
	}

	// Apply command line overrides to the tree
	for (const auto& kv_pair : override_settings) {
		user_settings.put(kv_pair.first, kv_pair.second);
	}

	try {
		register_messages();

		nanogui::init();

		Basestation session(user_settings);

		ScreenPositioning screen_cfg;
		screen_cfg.use_fullscreen = window_fullscreen;
		screen_cfg.size.x() = w;
		screen_cfg.size.y() = h;

		session.add_screen(new BasestationScreen(screen_cfg));
		
		session.mainloop();

		if (save_settings) {
			boost::property_tree::ptree settings;
			session.write_settings(settings);
			if (user_settings != settings) {
				std::cout << "Saving changes to settings.\n";
				write_settings(settings);
			}
		}

		nanogui::shutdown();

	} catch (const std::exception& e) {
		std::cerr << "Fatal error: Unhandled '" << typeid(e).name() << "' exception!\n\twhat(): " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
