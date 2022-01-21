add_subdirectory(can)
add_executable(subsystem subsystem.cpp drive_controller.hpp drive_controller.cpp)

target_compile_features(subsystem PRIVATE cxx_std_17)
