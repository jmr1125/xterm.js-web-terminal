cmake_minimum_required(VERSION 3.0)
project(WebTerminal)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost COMPONENTS system REQUIRED)

if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	message(STATUS "include_dirs = ${Boost_INCLUDE_DIRS}")
	message(STATUS "libs = ${Boost_LIBRARIES}")
	message(STATUS "ver = ${Boost_LIB_VERSION}")

endif()
add_executable(srv srv.cpp)
target_link_libraries(srv ${Boost_LIBRARIES})
