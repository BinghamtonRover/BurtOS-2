add_library(rover_can rover_can.hpp constants.hpp commands.hpp commands.cpp)

target_include_directories(rover_can PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(ONBOARD_CAN_BUS OFF CACHE BOOL "Compile subsystem with Linux CAN bus libraries")
set(CAN_LEN8_DLC OFF CACHE BOOL "Use when field name is can_frame::len8_dlc instead of can_frame::__res1")

if (ONBOARD_CAN_BUS)
	target_compile_definitions(rover_can PRIVATE ONBOARD_CAN_BUS)
	if (CAN_LEN8_DLC)
		target_compile_definitions(rover_can PRIVATE CAN_LEN8_DLC=len8_dlc)
	else()
		target_compile_definitions(rover_can PRIVATE CAN_LEN8_DLC=__res1)
	endif()
else()
	message("rover_can: Building in offline mode. CAN bus is disabled")
endif()
