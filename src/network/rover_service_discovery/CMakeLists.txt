find_package(Protobuf)
find_package(Boost)

if (Protobuf_FOUND AND Boost_FOUND)
	protobuf_generate_cpp(PROTO_SRC PROTO_HDR rsdp_functions.proto)

	add_library(rover_service_discovery rover_service_discovery.hpp rover_service_discovery.cpp ${PROTO_HDR} ${PROTO_SRC})
	target_include_directories(rover_service_discovery PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries(rover_service_discovery PUBLIC Protobuf Boost::boost)

	target_compile_features(rover_service_discovery PUBLIC cxx_std_17)

	if (WIN32)
		target_link_libraries(rover_service_discovery PRIVATE wsock32 ws2_32)
	endif()
else()
	message(STATUS "rover_service_discovery: dependencies not satisfied")
endif()

