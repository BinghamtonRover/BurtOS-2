add_executable(network network.cpp)

find_package(Boost REQUIRED)
if (WIN32)
	message("Boost target is WIN32")
	target_link_libraries(network wsock32 ws2_32)
else()
	message("Boost target is POSIX")
endif()

target_include_directories(network PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(network PUBLIC ${Boost_LIBRARIES})

find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRC PROTO_HDR hello.proto)

add_executable(protobuf_test protobuf_test.cpp ${PROTO_SRC} ${PROTO_HDR})
target_include_directories(protobuf_test PUBLIC ${Protobuf_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(protobuf_test PUBLIC ${Protobuf_LIBRARIES})
