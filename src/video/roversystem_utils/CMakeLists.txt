message(STATUS "roversystem_utils: the roversystem_utils library is deprecated. Please consider replacing.")
add_library(roversystem_utils STATIC include/roversystem/logger.hpp logger.cpp include/roversystem/simpleconfig.hpp simpleconfig.cpp include/roversystem/util.hpp)
target_include_directories(roversystem_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
