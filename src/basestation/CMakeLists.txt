add_executable(basestation basestation.cpp)

target_include_directories(basestation PUBLIC nanogui)
target_link_libraries(basestation nanogui ${NANOGUI_EXTRA_LIBS})

# NanoGUI requires C++17
target_compile_features(basestation PRIVATE cxx_std_17)