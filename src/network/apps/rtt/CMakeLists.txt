protobuf_generate_cpp(PROTO_SRC PROTO_HDR rtt_messages.proto)

find_package(Boost COMPONENTS program_options REQUIRED)
add_executable(rtt rtt.cpp ${PROTO_HDR} ${PROTO_SRC})
target_include_directories(rtt PUBLIC network ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(rtt PUBLIC network ${Boost_LIBRARIES})
