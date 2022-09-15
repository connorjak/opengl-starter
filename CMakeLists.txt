cmake_minimum_required (VERSION 3.8)

project ("opengl_starter")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory ("opengl_starter")

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/opengl_starter)