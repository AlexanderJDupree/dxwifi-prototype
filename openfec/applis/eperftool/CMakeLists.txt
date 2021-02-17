file (GLOB eperftool_sources ./*)

set(EPERFTOOL_BIN ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/eperftool CACHE STRING "eperftool dir")
add_executable( eperftool ${eperftool_sources})

set_target_properties(eperftool PROPERTIES COMPILE_FLAGS "-Wno-format -Wno-unused-result")

target_link_libraries( eperftool openfec m)


