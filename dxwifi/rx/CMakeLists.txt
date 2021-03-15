file(GLOB rx_sources ./*)

add_executable(rx ${rx_sources})

set_target_properties( rx 
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${DXWIFI_RUNTIME_OUTPUT_DIRECTORY}
    )

target_link_libraries(rx dxwifi)
