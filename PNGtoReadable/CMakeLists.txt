cmake_minimum_required(VERSION 3.16)
project(PNGtoBitmap C)

set(CMAKE_C_STANDARD 11)
add_executable(PNGtoBitmap main.c spng/spng.c spng/spng.h)

target_link_libraries(PNGtoBitmap z)
target_link_libraries(PNGtoBitmap m)
