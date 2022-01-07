cmake_minimum_required(VERSION 3.16)

project(BurtOS-2 DESCRIPTION "Base station and rover computer software for Binghamton University Rover Team (BURT)")

# Output binary files in more accessible locations than in the source subdirectories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(src/network)

# The graphics libraries for the base station will not be present on the rover subsystem computers
# Default value is OFF (build everything) but should be overridden when building on a Raspberry Pi
set(NO_GFX OFF CACHE BOOL "Exclude all graphics libraries and the base station")

if (NOT NO_GFX)
	find_package(OpenGL)
	if (OPENGL_FOUND)
		if (NOT EXISTS ext/nanogui/CMakeLists.txt AND NOT NO_GFX)
			message(FATAL_ERROR
				"Base station required submodules are missing! Either explicilty set "
				"NO_GFX=ON to exclude the base station or clone all submodules (run:"
				" git submodule update --init --recursive)"
			)
		endif()
	elseif(NOT NO_GFX)
		message(WARNING
			"No graphics support. Excluding base station and building in onboard mode. To"
			" disable this warning, explicitly set NO_GFX=ON"
		)
	endif()
endif()

if (NOT NO_GFX AND OPENGL_FOUND)
	set(NANOGUI_BUILD_EXAMPLES	OFF CACHE BOOL " " FORCE)
	set(NANOGUI_BUILD_PYTHON	OFF CACHE BOOL " " FORCE)
	set(NANOGUI_INSTALL			OFF CACHE BOOL " " FORCE)
	set(NANOGUI_BUILD_SHARED	OFF CACHE BOOL " " FORCE)
	add_subdirectory(ext/nanogui)
	set_property(TARGET glfw glfw_objects nanogui PROPERTY FOLDER "dependencies")

	add_subdirectory(src/basestation)
else()
	message(STATUS "NO_GFX is ON. Base Station is excluded from this build.")
endif()


add_subdirectory(src/subsystem)
