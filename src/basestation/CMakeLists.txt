add_executable(basestation basestation.cpp session.hpp session.cpp window_module.hpp window_module.cpp stacked_widget.hpp stacked_widget.cpp)

target_include_directories(basestation PUBLIC nanogui)
target_link_libraries(basestation nanogui ${NANOGUI_EXTRA_LIBS})

# NanoGUI requires C++17
target_compile_features(basestation PRIVATE cxx_std_17)

if (APPLE)
	target_link_libraries(basestation "-framework OpenGL")
endif()
