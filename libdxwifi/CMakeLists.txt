cmake_minimum_required(VERSION 3.2)
project(libdxwifi)

# Add new files here!
add_library(dxwifi STATIC 
            transmitter.c
            receiver.c
            ${PROJECT_SOURCE_DIR}/details/heap.c
            ${PROJECT_SOURCE_DIR}/details/logging.c
            )   

set_target_properties(dxwifi PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Wno-unused-function")

target_link_libraries(dxwifi pcap)
