add_subdirectory(can)
add_subdirectory(task_scheduler)

find_package(Boost COMPONENTS system)
if (Boost_FOUND AND Boost_system_FOUND)
	add_executable(subsystem subsystem.cpp drive_controller.hpp drive_controller.cpp)
	target_include_directories(subsystem PUBLIC ${Boost_INCLUDE_DIR} rover_can rover_system_messages)
	target_link_libraries(subsystem PUBLIC network rover_can rover_system_messages Boost::system)

	target_compile_features(subsystem PRIVATE cxx_std_17)
else()
	message(STATUS "rover_subsystem: excluded from build since Boost is missing")
endif()
