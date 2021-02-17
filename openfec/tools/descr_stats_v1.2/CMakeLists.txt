file (GLOB descr_stat_sources ./descr_stats.c)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/perf_eval)
add_executable(descr_stats  ${descr_stat_sources})

set_target_properties(descr_stats PROPERTIES COMPILE_FLAGS "-Wno-unused-result -Wno-format-overflow")

target_link_libraries( descr_stats  m)
