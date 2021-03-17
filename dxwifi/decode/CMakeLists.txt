file(GLOB decode_sources ./*)

add_executable(decode ${decode_sources})

set_target_properties(decode
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${DXWIFI_RUNTIME_OUTPUT_DIRECTORY}
    )

target_link_libraries(decode dxwifi)
