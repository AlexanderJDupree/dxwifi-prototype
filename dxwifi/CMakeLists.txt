cmake_minimum_required(VERSION 3.2)
project(dxwifi)

add_executable(tx
               tx.c
               cli.c
               )

target_link_libraries(tx libdxwifi)
