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

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g -D_DEBUG -Wall")


enable_testing()

# Test build instructions for check unit tests
FUNCTION(ADD_CHECK_TEST TEST_NAME)
    add_executable(${TEST_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/tests/${TEST_NAME}.cpp)
    target_include_directories(${TEST_NAME} PRIVATE include)
    target_link_libraries(${TEST_NAME} PUBLIC check subunit pthread ${ARGN})
ENDFUNCTION()

if (tests)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/tests")
    make_directory(${CMAKE_CURRENT_SOURCE_DIR}/bin/tests/data)

    ADD_CHECK_TEST(pagedfile_tests gsl)
    ADD_CHECK_TEST(progressbar_tests)
    ADD_CHECK_TEST(static_isam_tests gsl)
    ADD_CHECK_TEST(bentley_saxe_tests gsl)
    ADD_CHECK_TEST(priority_queue_tests)
    ADD_CHECK_TEST(bitarray_check_tests)
    ADD_CHECK_TEST(bloomfilter_tests gsl)
    ADD_CHECK_TEST(alias_tests gsl)
endif()


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
