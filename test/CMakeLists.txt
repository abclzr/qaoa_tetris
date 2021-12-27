macro(package_add_test_with_libraries TESTNAME FILES TEST_WORKING_DIRECTORY)
    set(LIBRARIES ${ARGN})
    add_executable(${TESTNAME} ${FILES})
    target_link_libraries(${TESTNAME} gtest gmock gtest_main ${LIBRARIES})
    gtest_discover_tests(${TESTNAME}
        WORKING_DIRECTORY ${TEST_WORKING_DIRECTORY}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${TEST_WORKING_DIRECTORY}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER test)
endmacro()

package_add_test_with_libraries(graph_test graph_test.cpp "${PROJECT_SOURCE_DIR}/test/graph" graph)

package_add_test_with_libraries(cs_test cs_test.cpp "${PROJECT_SOURCE_DIR}/test/graph" graph graph_match )


# Test
# package_add_test(graph_test graph_test.cpp)
# target_link_libraries(graph_test graph gtest gmock_main)

# # Make sure third-party is built before executable
# add_dependencies(graph_test googletest gmock_main)
# set_target_properties(graph_test PROPERTIES EXCLUDE_FROM_ALL TRUE)