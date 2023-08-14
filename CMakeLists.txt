cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project (webterm)

message(CHECK_START "Finding libuv")
find_path(LIBUV_INCLUDE_DIR uv.h)
find_library(LIBUV_LIBRARY NAMES uv_a uv)

message(STATUS "INCLUDE DIR:${LIBUV_INCLUDE_DIR}")
message(STATUS "LIBRARY DIR:${LIBUV_LIBRARY}")

if(NOT(("${LIBUV_INCLUDE_DIR}" EQUAL "") OR ("${LIBUV_LIBRARY}" EQUAL "")))
	message(CHECK_PASS "found")
	set(LIBUV_FOUND ON)
else()
	message(CHECK_FAIL "not found")
	SET(LIBUV_FOUND OFF)
endif()

if(${LIBUV_FOUND})
  add_subdirectory(libuv)
endif()
find_package(Boost COMPONENTS system REQUIRED)

if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	message(STATUS "include_dirs = ${Boost_INCLUDE_DIRS}")
	message(STATUS "libs = ${Boost_LIBRARIES}")
	message(STATUS "ver = ${Boost_LIB_VERSION}")

      endif()
      add_subdirectory(boost)
endif()

