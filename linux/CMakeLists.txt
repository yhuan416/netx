cmake_minimum_required(VERSION 3.0)

file(GLOB srcs "*.c")

add_library(netxlinux STATIC ${srcs})
target_include_directories(netxlinux PRIVATE ../)
target_include_directories(netxlinux PUBLIC .)
target_link_libraries(netxlinux PRIVATE pthread)
