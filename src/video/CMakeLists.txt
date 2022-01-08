if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

	set(PKG_TURBOJPEG_PATH "/opt/libjpeg-turbo/lib64/pkgconfig" CACHE STRING "Search path for libjpeg-turbo pkg-config files")
	set(ENV{PKG_CONFIG_PATH} PKG_TURBOJPEG_PATH)

	find_package(PkgConfig)
	if (NOT PKG_CONFIG_FOUND)
		message(STATUS "video: pkg-config unavailable.")
	else()
		pkg_search_module(PKG_LIBJPEG_TURBO IMPORTED_TARGET libturbojpeg)

		if (NOT PKG_LIBJPEG_TURBO_FOUND)
			message(STATUS "video: libjpeg-turbo unavailable.")
		else()

			add_subdirectory(roversystem_utils)
			add_executable(video camera.hpp camera.cpp session.hpp session.cpp main.cpp)
			target_link_libraries(video roversystem_utils PkgConfig::PKG_LIBJPEG_TURBO)
			set(VIDEO_BUILT ON)

		endif()
	endif()
else()
	message(STATUS "video: Video4Linux unavailable. Only Linux-based systems are supported.")
endif()

if (NOT DEFINED VIDEO_BUILT)
	message(STATUS "video: Not building video program.")
else()
	message(STATUS "video: Dependencies satisfied. Building video program.")
endif()
