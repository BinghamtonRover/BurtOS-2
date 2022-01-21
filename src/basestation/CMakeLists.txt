set(MODULE_HDR
	modules/console.hpp
)
set(MODULE_SRC
	modules/console.cpp
)

set(WIDGET_HDR
	widgets/textarea.hpp
	widgets/functionbox.hpp
)
set(WIDGET_SRC
	widgets/textarea.cpp
	widgets/functionbox.cpp
)

add_executable(basestation basestation.cpp session.hpp session.cpp ${MODULE_HDR} ${MODULE_SRC} ${WIDGET_HDR} ${WIDGET_SRC})

target_include_directories(basestation PUBLIC nanogui roverlua)
target_link_libraries(basestation nanogui ${NANOGUI_EXTRA_LIBS} roverlua)

# NanoGUI requires C++17
target_compile_features(basestation PRIVATE cxx_std_17)

if (APPLE)
	target_link_libraries(basestation "-framework OpenGL")
endif()
