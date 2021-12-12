add_library(rover_can rover_can.hpp constants.hpp commands.hpp commands.cpp)

target_include_directories(rover_can PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (NOT DEFINED ONBOARD_CAN_BUS)
	if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		set(__AUTO_DETECT_CAN ON BOOL)
		message(STATUS "rover_can: Building with full CAN functionality")
	else()
		set(__AUTO_DETECT_CAN OFF BOOL)
		message(STATUS "rover_can: Platform does not support rover CAN bus")
	endif()
endif()

set(ONBOARD_CAN_BUS __AUTO_DETECT_CAN CACHE BOOL "Compile subsystem with Linux CAN bus libraries")


if (ONBOARD_CAN_BUS)
	target_compile_definitions(rover_can PRIVATE ONBOARD_CAN_BUS)
else()
	message(STATUS "rover_can: Building in offline mode. CAN bus is disabled")
endif()
