cmake_minimum_required(VERSION 3.0)

project(Netx C)

add_executable(netx main.c netx.c netx_uart.c)
target_include_directories(netx PRIVATE .)
target_link_libraries(netx PRIVATE pthread)
