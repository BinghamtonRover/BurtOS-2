add_library(rover_can rover_can.hpp constants.hpp commands.hpp commands.cpp)

target_include_directories(rover_can PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# If ONBOARD_CAN_BUS has been manually defined (or is cached), then don't override it with auto-detection
if (NOT DEFINED ONBOARD_CAN_BUS)
	if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		set(ONBOARD_CAN_BUS ON CACHE BOOL "Compile subsystem with Linux CAN bus libraries")
		message(STATUS "rover_can: Platform supports full CAN functionality")
	else()
		set(ONBOARD_CAN_BUS OFF CACHE BOOL "Compile subsystem with Linux CAN bus libraries")
		message(STATUS "rover_can: Platform does not support rover CAN bus")
	endif()
endif()

if (ONBOARD_CAN_BUS)
	target_compile_definitions(rover_can PRIVATE ONBOARD_CAN_BUS)
	message(STATUS "rover_can: Building in ONBOARD mode")
else()
	message(STATUS "rover_can: Building in OFFLINE mode. CAN bus is disabled")
endif()
