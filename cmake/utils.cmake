cmake_minimum_required(VERSION 3.21..4.4)

function(veer_compile_options TARGET)
    set_property(TARGET ${TARGET}
        PROPERTY CXX_STANDARD 26
    )
    target_compile_options(${TARGET}
    PUBLIC
        -std=c++26
        -freflection
    PRIVATE
        -ffast-math
        -Wall
        -fno-omit-frame-pointer
        -static-libasan
    )
endfunction()

function(veer_include_directories TARGET)
    target_include_directories(${TARGET}
    PRIVATE
        include/veer_${TARGET}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
endfunction()

include(GNUInstallDirs)
function(veer_target_install TARGET)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/veer_${TARGET}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/veer
    )
endfunction()

function(veer_target_common TARGET)
    veer_compile_options(${TARGET})
    veer_include_directories(${TARGET})
    veer_target_install(${TARGET})
endfunction()
