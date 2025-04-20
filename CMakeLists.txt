cmake_minimum_required(VERSION 3.29)
project(seekify)

set(CMAKE_CXX_STANDARD 20)

add_executable(seekify main.cpp Graph.cpp)
set(FLTK_SOURCE_DIR "${PROJECT_SOURCE_DIR}/fltk-1.4.2-source/fltk/fltk-1.4.2")
add_subdirectory(${FLTK_SOURCE_DIR})
include_directories("${FLTK_SOURCE_DIR}/CMake")
target_link_libraries(seekify PRIVATE fltk::fltk)
