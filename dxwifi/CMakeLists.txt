cmake_minimum_required(VERSION 3.2)
project(dxwifi)

add_executable(tx
               tx.c
               )

add_executable(rx
               rx.c
               )

target_link_libraries(tx dxwifi)
target_link_libraries(rx dxwifi)
