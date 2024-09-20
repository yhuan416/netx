cmake_minimum_required(VERSION 3.0)

project(Netx C)

# add_compile_options(-ggdb)

add_subdirectory(linux)

add_library(netx STATIC netx.c)
target_include_directories(netx PUBLIC .)

add_executable(main main.c)
target_link_libraries(main PRIVATE netx netxlinux)
