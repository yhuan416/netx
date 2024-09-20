cmake_minimum_required(VERSION 3.0)

file(GLOB srcs "*.c")
file(GLOB common_srcs "common/*.c")

add_library(netxlinux STATIC ${srcs} ${common_srcs})
target_include_directories(netxlinux PRIVATE ../ ./common)
target_include_directories(netxlinux PUBLIC .)
target_link_libraries(netxlinux PRIVATE pthread)
