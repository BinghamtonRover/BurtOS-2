protobuf_generate_cpp(PROTO_SRC PROTO_HDR chat_messages.proto)

find_package(Boost COMPONENTS program_options REQUIRED)
add_executable(chat chat.cpp ${PROTO_HDR} ${PROTO_SRC})
target_include_directories(chat PUBLIC network ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(chat PUBLIC network ${Boost_LIBRARIES})