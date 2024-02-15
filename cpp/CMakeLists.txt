cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(namespace "psudb-common")
project("Penn State University Database Group Common Libraries" VERSION 0.1.0)

set(debug true)
set(tests true)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

if (debug) 
    add_compile_options(-g -O0)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
else()
    add_compile_options(-O3)
endif()


# Test build instructions
if (tests) 
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/tests")
    make_directory(bin/tests/data)

    add_executable(pagedfile_tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/pagedfile_tests.cpp)
    target_link_libraries(pagedfile_tests PUBLIC check subunit pthread gsl)
    target_include_directories(pagedfile_tests PRIVATE include external/psudb-common/include)

    add_executable(progressbar_tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/progressbar_tests.cpp)
    target_link_libraries(progressbar_tests PUBLIC check subunit pthread gsl)
    target_include_directories(progressbar_tests PRIVATE include external/psudb-common/include)
endif()
