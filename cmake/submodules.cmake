cmake_minimum_required(VERSION 3.15..4.0)

file(GLOB GLOB_DIRS LIST_DIRECTORIES true extern/*)
set(VEER_SUBMODULES "")
foreach(GLOB_DIR ${GLOB_DIRS})
    if(IS_DIRECTORY ${GLOB_DIR})
        list(APPEND VEER_SUBMODULES ${GLOB_DIR})
    endif()
endforeach()

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

foreach(SUBMODULE ${VEER_SUBMODULES})
    message("Verifying submodule ${SUBMODULE}...")
    if(NOT EXISTS "${SUBMODULE}/CMakeLists.txt")
        message(FATAL_ERROR "The submodule ${SUBMODULE} was not downloaded correctly! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
    endif()
endforeach()

set(SDL_STATIC ON)
set(SDL_SHARED OFF)
set(SDL_TEST_LIBRARY OFF)
set(SDL_TESTS OFF)
add_subdirectory(extern/SDL3)

set(GLM_ENABLE_CXX_20 ON)
set(GLM_BUILD_SHARED_LIBS OFF)
add_subdirectory(extern/glm)

