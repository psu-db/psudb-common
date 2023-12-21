

FUNCTION(ADD_TEST TEST_NAME HEADERS LIBRARY)
    add_executable(
            ${TEST_NAME} ${TEST_NAME}.cpp ${HEADERS}
    )
    target_include_directories(${TEST_NAME} PUBLIC ../include/${LIBRARY})
    target_link_libraries(
            ${TEST_NAME}
            ${LIBRARY}
            GTest::gtest_main
    )
    gtest_discover_tests(${TEST_NAME})

ENDFUNCTION()

include(GoogleTest)

ADD_TEST(dynarray_tests "" psu-ds)