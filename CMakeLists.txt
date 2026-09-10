cmake_minimum_required(VERSION 3.21..4.4)

project(
    veer
    VERSION 0.0.1
    DESCRIPTION "ECS engine whatever"
    LANGUAGES C CXX
)

if (PROJECT_IS_TOP_LEVEL)
    message("Veer is the top-level project")
    set(VEER_IS_TOP_LEVEL ON)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(${PROJECT_SOURCE_DIR}/cmake/submodules.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/utils.cmake)

find_package(Vulkan REQUIRED)
find_package(slang REQUIRED)

add_library(veer INTERFACE)

set_property(TARGET veer
    PROPERTY CXX_STANDARD 26
)

set(
    VEER_COMPONENTS
        veer_core
        veer_render
)

foreach(VEER_COMPONENT ${VEER_COMPONENTS})
    add_subdirectory(${VEER_COMPONENT})
endforeach()

target_link_libraries(
    veer
    INTERFACE
        ${VEER_COMPONENTS}
)

target_include_directories(veer
    INTERFACE
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/veer>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/veer/extern>
)

option(VEER_INSTALL "Install veer" ${VEER_IS_TOP_LEVEL})
if (${VEER_INSTALL})
    message("Configuring veer install...")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/veerConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/veerConfig.cmake
        COPYONLY
    )
    install(TARGETS veer ${VEER_COMPONENTS}
        EXPORT veerTargets
    )
    install(EXPORT veerTargets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/veer/cmake
        NAMESPACE veer::
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/veerConfig.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/veer/cmake
    )
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/extern/glm/glm
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/veer/extern
    )
    export(TARGETS veer ${VEER_COMPONENTS}
        NAMESPACE veer::
        FILE veerTargets.cmake
    )
endif()

if (${VEER_IS_TOP_LEVEL})
    message("Will install veer after build")
    add_custom_target(
        veer_forced_install ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/__dummy_file
    )
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/__dummy_file
        COMMENT "Installing veer to ~/.local"
        COMMAND ./do_install.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_dependencies(veer_forced_install ${VEER_COMPONENTS})
endif()

option(VEER_TESTS "Build tests" ON)
if (${VEER_TESTS})
    message("Configuring veer tests...")
    add_subdirectory(tests)
endif()

