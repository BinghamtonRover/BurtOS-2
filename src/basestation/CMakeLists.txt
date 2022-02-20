set(MODULE_HDR
	modules/console.hpp
	modules/network_settings.cpp
	modules/drive_stats.hpp
	modules/input_config/controller_config.hpp
	modules/input_config/controller_calibration_popup.hpp
	modules/input_config/controller_binding_popup.hpp
)
set(MODULE_SRC
	modules/console.cpp
	modules/network_settings.cpp
	modules/drive_stats.cpp
	modules/input_config/controller_config.cpp
	modules/input_config/controller_calibration_popup.cpp
	modules/input_config/controller_binding_popup.cpp
)

set(WIDGET_HDR
	widgets/textarea.hpp
	widgets/functionbox.hpp
)
set(WIDGET_SRC
	widgets/textarea.cpp
	widgets/functionbox.cpp
)

add_executable(basestation
	main.cpp
	basestation.hpp
	basestation.cpp
	basestation_screen.hpp
	basestation_screen.cpp
	controls/controller_manager.hpp
	controls/controller_manager.cpp
	controls/controller.hpp
	controls/controller.cpp
	controls/lua_ctrl_lib.hpp
	controls/lua_ctrl_lib.cpp
	controls/drive_input.hpp
	controls/drive_input.cpp
	${MODULE_HDR}
	${MODULE_SRC}
	${WIDGET_HDR}
	${WIDGET_SRC} 
)

find_package(Boost COMPONENTS program_options REQUIRED)

target_include_directories(basestation PUBLIC nanogui roverlua ${CMAKE_CURRENT_SOURCE_DIR} rover_control Boost::headers)
target_link_libraries(basestation nanogui ${NANOGUI_EXTRA_LIBS} roverlua rover_control Boost::program_options)

# NanoGUI requires C++17
target_compile_features(basestation PUBLIC cxx_std_17)

if (APPLE)
	target_link_libraries(basestation "-framework OpenGL")
endif()
