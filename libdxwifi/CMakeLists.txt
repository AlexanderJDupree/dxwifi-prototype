cmake_minimum_required(VERSION 3.2)
project(libdxwifi)

# Add new files here!
add_library(libdxwifi STATIC 
            dxwifi.c
            )   

set_target_properties(libdxwifi PROPERTIES COMPILE_FLAGS "-Wall -Wextra")

target_link_libraries(libdxwifi pcap)
