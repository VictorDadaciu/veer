cmake_minimum_required(VERSION 3.21..4.4)

find_package(veer PATHS ~/.local)

if (${VEER_FOUND})
    project(arena)

    add_executable(
        arena
        main.cpp
    )
    add_dependencies(arena veer_forced_install)
                
    set_property(
        TARGET arena
        PROPERTY CXX_STANDARD 26
    )

    target_compile_options(
        arena PRIVATE
        -ffast-math
        -fsanitize=address
        -fno-omit-frame-pointer
        -static-libasan
        -Wall
    )

    target_link_options(
        arena PRIVATE
        -fsanitize=address
    )

    target_link_libraries(
        arena
        PRIVATE
        veer::veer
    )
    
    add_dependencies(arena veer_forced_install)
endif()
