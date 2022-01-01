find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRC PROTO_HDR extras.proto)

add_library(network network.hpp network.cpp network_util.hpp ${PROTO_SRC} ${PROTO_HDR})

find_package(Boost 1.71.0 REQUIRED)
if (WIN32)
	message(STATUS "Boost target is WIN32")
	target_link_libraries(network PUBLIC wsock32 ws2_32)
else()
	message(STATUS "Boost target is POSIX")
endif()

target_include_directories(network PUBLIC ${Boost_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Protobuf_INCLUDE_DIRS})
target_link_libraries(network PUBLIC ${Boost_LIBRARIES} ${Protobuf_LIBRARIES})

add_subdirectory(apps)
