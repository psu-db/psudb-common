cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(namespace "psudb-common")
project("Penn State University Database Group Common Libraries" VERSION 0.1.0)

set(debug true)
set(tests true)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

#[[if (debug)
    add_compile_options(-g -O0)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
else()
    add_compile_options(-O3)
endif()]]

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g -D_DEBUG")


# Test build instructions for check unit tests
if (tests) 
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/tests")
    make_directory(bin/tests/data)

    add_executable(pagedfile_tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/pagedfile_tests.cpp)
    target_link_libraries(pagedfile_tests PUBLIC check subunit pthread gsl cblas)
    target_include_directories(pagedfile_tests PRIVATE include external/psudb-common/include)

    add_executable(progressbar_tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/progressbar_tests.cpp)
    target_link_libraries(progressbar_tests PUBLIC check subunit pthread gsl cblas)
    target_include_directories(progressbar_tests PRIVATE include external/psudb-common/include)

    add_executable(static_isam_tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/static_isam_tests.cpp)
    target_link_libraries(static_isam_tests PUBLIC check subunit pthread gsl cblas)
    target_include_directories(static_isam_tests PRIVATE include external/psudb-common/include)

    add_executable(bentley_saxe_tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/bentley_saxe_tests.cpp)
    target_link_libraries(bentley_saxe_tests PUBLIC check subunit pthread gsl cblas)
    target_include_directories(bentley_saxe_tests PRIVATE include external/psudb-common/include)
endif()

enable_testing()


# google test
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_subdirectory(include/psu-ds)
add_subdirectory(include/psu-util)

add_subdirectory(tests)
