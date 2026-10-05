cmake_minimum_required(VERSION 3.21..4.4)

add_subdirectory(arena)

add_executable(unit_tests test_runner.cpp)

target_include_directories(
    unit_tests
    PRIVATE
        ../extern/cppunit   
)

target_link_libraries(
    unit_tests
    PRIVATE
        veer
)

veer_compile_options(unit_tests)

add_dependencies(unit_tests ${VEER_COMPONENTS})

option(VEER_RUN_TESTS "Run tests after build" ${VEER_IS_TOP_LEVEL})
if (${VEER_RUN_TESTS})
    message("Will run tests after build")
    add_custom_target(
        veer_run_tests ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/__dummy_file
    )
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/__dummy_file
        COMMENT "Running tests..."
        COMMAND ./../build/tests/unit_tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_dependencies(veer_run_tests unit_tests)
endif()
