file(GLOB tx_sources ./*)

add_executable(tx ${tx_sources})

set_target_properties( tx 
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${DXWIFI_RUNTIME_OUTPUT_DIRECTORY}
    )

target_link_libraries(tx dxwifi)
