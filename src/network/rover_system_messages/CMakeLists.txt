protobuf_generate_cpp(PROTO_SRC PROTO_HDR video_control.proto drive_control.proto)

add_library(rover_system_messages rover_system_messages.hpp ${PROTO_SRC} ${PROTO_HDR}) 
target_include_directories(rover_system_messages PUBLIC network ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(rover_system_messages PUBLIC network)
