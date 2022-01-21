add_subdirectory(can)
add_executable(subsystem subsystem.cpp drive_controller.hpp drive_controller.cpp)

find_package(Boost REQUIRED)
target_include_directories(subsystem PRIVATE ${Boost_INCLUDE_DIR})

target_compile_features(subsystem PRIVATE cxx_std_17)
