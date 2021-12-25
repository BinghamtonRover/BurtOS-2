add_library(network network.hpp network.cpp network_util.hpp)

find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRC PROTO_HDR hello.proto)

find_package(Boost REQUIRED)
if (WIN32)
	message(STATUS "Boost target is WIN32")
	target_link_libraries(network PUBLIC wsock32 ws2_32)
else()
	message("Boost target is POSIX")
endif()

target_include_directories(network PUBLIC ${Boost_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${Protobuf_INCLUDE_DIRS})
target_link_libraries(network PUBLIC ${Boost_LIBRARIES} ${Protobuf_LIBRARIES})

add_executable(network_test network_test.cpp ${PROTO_SRC} ${PROTO_HDR})
target_include_directories(network_test PUBLIC ${Protobuf_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} network)
target_link_libraries(network_test PUBLIC ${Protobuf_LIBRARIES} network)
