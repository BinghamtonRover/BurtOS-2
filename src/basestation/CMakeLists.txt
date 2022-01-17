add_executable(basestation basestation.cpp session.hpp session.cpp modules/console.hpp widgets/action_textbox.hpp)

target_include_directories(basestation PUBLIC nanogui roverlua)
target_link_libraries(basestation nanogui ${NANOGUI_EXTRA_LIBS} roverlua)

# NanoGUI requires C++17
target_compile_features(basestation PRIVATE cxx_std_17)

if (APPLE)
	target_link_libraries(basestation "-framework OpenGL")
endif()
