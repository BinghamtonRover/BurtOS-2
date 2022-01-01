set(BUILD_NETWORK_APPS OFF BOOL "Build extra example/testing applications for the network library")

if (BUILD_NETWORK_APPS)

	add_executable(network_test network_test.cpp)
	target_include_directories(network_test PUBLIC network)
	target_link_libraries(network_test PUBLIC network)

	find_package(Boost COMPONENTS program_options REQUIRED)
	add_executable(rtt rtt.cpp ${network_required_src})
	target_include_directories(rtt PUBLIC network ${Boost_INCLUDE_DIRS})
	target_link_libraries(rtt PUBLIC network ${Boost_LIBRARIES})

endif()
