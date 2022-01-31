macro(package_add_test_with_libraries TESTNAME FILES TEST_WORKING_DIRECTORY)
    set(LIBRARIES ${ARGN})
    add_executable(${TESTNAME} ${FILES})
    target_link_libraries(${TESTNAME} gtest gmock gtest_main ${LIBRARIES})
    gtest_discover_tests(${TESTNAME}
        WORKING_DIRECTORY "${TEST_WORKING_DIRECTORY}"
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${TEST_WORKING_DIRECTORY}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER test)
    target_compile_definitions(${TESTNAME} PUBLIC TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/testcases/")
endmacro()

package_add_test_with_libraries(bimap_test bimap_test.cpp "${CMAKE_SOURCE_DIR}" bimap)
