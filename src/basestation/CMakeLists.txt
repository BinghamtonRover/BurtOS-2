set(MODULE_HDR
	modules/console.hpp
	modules/console.hpp
	modules/video_decoder/decoder.hpp
	modules/input_config/controller_config.hpp
	modules/input_config/controller_calibration_popup.hpp
	modules/input_config/controller_binding_popup.hpp
	modules/video_feed_viewer.hpp
)
set(MODULE_SRC
	modules/console.cpp
	modules/video_feed_viewer.cpp
	modules/video_decoder/decoder.cpp
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

find_package(PkgConfig)
if (NOT PKG_CONFIG_FOUND)
	message(STATUS "basestation: pkg-config unavailable.")
else()
	pkg_search_module(PKG_LIBJPEG_TURBO IMPORTED_TARGET libturbojpeg)

	if (NOT PKG_LIBJPEG_TURBO_FOUND)
		message(STATUS "basestation: libjpeg-turbo unavailable.")
	else()

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
			${MODULE_HDR}
			${MODULE_SRC}
			${WIDGET_HDR}
			${WIDGET_SRC}
		)

		target_include_directories(basestation PUBLIC nanogui roverlua network ${CMAKE_CURRENT_SOURCE_DIR})
		target_link_libraries(basestation nanogui ${NANOGUI_EXTRA_LIBS} roverlua network PkgConfig::PKG_LIBJPEG_TURBO)

		# NanoGUI requires C++17
		target_compile_features(basestation PUBLIC cxx_std_17)

		if (APPLE)
			target_link_libraries(basestation "-framework OpenGL")
		endif()

	endif()
endif()


