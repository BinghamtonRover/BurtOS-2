set(MODULE_HDR
	modules/console.hpp
	modules/input_config/controller_config.hpp
	modules/input_config/controller_calibration_popup.hpp
	modules/input_config/controller_binding_popup.hpp
)
set(MODULE_SRC
	modules/console.cpp
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

add_executable(basestation basestation.cpp session.hpp session.cpp ${MODULE_HDR} ${MODULE_SRC} ${WIDGET_HDR} ${WIDGET_SRC} 
	controls/controller_manager.hpp
	controls/controller_manager.cpp
	controls/controller.hpp
	controls/controller.cpp
	controls/lua_ctrl_lib.hpp
	controls/lua_ctrl_lib.cpp
)

target_include_directories(basestation PUBLIC nanogui roverlua ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(basestation nanogui ${NANOGUI_EXTRA_LIBS} roverlua)

# NanoGUI requires C++17
target_compile_features(basestation PUBLIC cxx_std_17)

if (APPLE)
	target_link_libraries(basestation "-framework OpenGL")
endif()
