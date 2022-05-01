
# If ROVER_CAN_AVAIL has been manually defined (or is cached), then don't override it with auto-detection
if (NOT DEFINED ROVER_CAN_AVAIL)
	if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		set(ROVER_CAN_AVAIL ON CACHE BOOL "Compile subsystem with Linux CAN bus libraries")
		message(STATUS "rover_can: Platform supports full CAN functionality")
	else()
		set(ROVER_CAN_AVAIL OFF CACHE BOOL "Compile subsystem with Linux CAN bus libraries")
		message(STATUS "rover_can: Platform does not support rover CAN bus")
	endif()
endif()

add_library(rover_can rover_can.hpp constants.hpp commands.hpp commands.cpp)

target_include_directories(rover_can PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rover_can PRIVATE cxx_std_20)

if (ROVER_CAN_AVAIL)
	target_compile_definitions(rover_can PUBLIC ROVER_CAN_AVAIL)
	message(STATUS "rover_can: Building with full CAN bus support")
else()
	message(STATUS "rover_can: Building in placeholder/OFFLINE mode. CAN bus is disabled")
endif()