add_executable(rover_control rover_control.cpp rover_control.hpp)

find_package(Boost REQUIRED)
target_include_directories(subsystem PUBLIC ${Boost_INCLUDE_DIR} rover_system_messages)
target_link_libraries(subsystem PUBLIC network rover_system_messages)

target_compile_features(subsystem PRIVATE cxx_std_17)
