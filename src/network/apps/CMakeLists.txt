set(BUILD_NETWORK_APPS OFF CACHE BOOL "Build extra example/testing applications for the network library")

if (BUILD_NETWORK_APPS)
	message(STATUS "Building extra network applications")
	add_subdirectory(chat)
	add_subdirectory(rtt)
endif()
