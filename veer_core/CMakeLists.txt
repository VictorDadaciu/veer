cmake_minimum_required(VERSION 3.21..4.4)

set(VEER_CORE_SOURCES
    src/log.cpp
)

set(VEER_CORE_HEADERS
    include/veer_core/db.h
    include/veer_core/error_code.h
    include/veer_core/log.h
    include/veer_core/utils.h
)

add_library(veer_core STATIC
    ${VEER_CORE_SOURCES}
    ${VEER_CORE_HEADERS}
)

target_include_directories(
    veer_core
    PRIVATE
        ../extern/glm
)

veer_target_common(veer_core)
