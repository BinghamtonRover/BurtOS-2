find_package(Protobuf REQUIRED)
add_library(network network.hpp network.cpp network_util.hpp messages.hpp messages.cpp stream.hpp stream.cpp)

find_package(Boost REQUIRED)
if (WIN32)
	message(STATUS "Boost target is WIN32")
	target_link_libraries(network PUBLIC wsock32 ws2_32)
else()
	message(STATUS "Boost target is POSIX")
endif()

target_include_directories(network PUBLIC ${Boost_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Protobuf_INCLUDE_DIRS})
target_link_libraries(network PUBLIC ${Boost_LIBRARIES} ${Protobuf_LIBRARIES} events)
target_compile_features(network PUBLIC cxx_std_17)

add_subdirectory(apps)
add_subdirectory(rover_system_messages)
add_subdirectory(rover_service_discovery)
