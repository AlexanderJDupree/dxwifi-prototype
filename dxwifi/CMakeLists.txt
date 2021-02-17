cmake_minimum_required(VERSION 3.2)
project(dxwifi)

add_executable(tx
               tx.c
               )

add_executable(rx
               rx.c
               )

add_executable(encode
               encode.c
               )               

target_link_libraries(tx dxwifi)
target_link_libraries(rx dxwifi)
target_link_libraries(encode dxwifi openfec)
