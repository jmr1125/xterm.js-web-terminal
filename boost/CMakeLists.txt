cmake_minimum_required(VERSION 3.0)
add_executable(srv srv.cpp)
target_link_libraries(srv ${Boost_LIBRARIES})
