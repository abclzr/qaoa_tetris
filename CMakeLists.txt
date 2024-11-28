cmake_minimum_required(VERSION 3.1.1)
project(subiso VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(GRAPH__ENABLE_TESTS OFF)
set(BIMAP__ENABLE_TESTS OFF)

set(QAOAGRAPH_SOURCE_DIR "${PROJECT_SOURCE_DIR}/submodule/QAOAGraph")
set(BIMAP_SOURCE_DIR "${PROJECT_SOURCE_DIR}/submodule/BiMap")


if (NOT TARGET graph)
        add_subdirectory(${QAOAGRAPH_SOURCE_DIR} EXCLUDE_FROM_ALL)
endif()

if (NOT TARGET bimap)
        add_subdirectory(${BIMAP_SOURCE_DIR} EXCLUDE_FROM_ALL)
endif()

add_library(graph_match src/GraphMatch.cpp)

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${QAOAGRAPH_SOURCE_DIR}")
include_directories("${BIMAP_SOURCE_DIR}")

target_link_libraries(graph_match graph bimap)

add_executable(subiso main.cpp)
add_executable(tetris tetris_qaoa.cpp)
target_link_libraries(subiso graph graph_match bimap)
target_link_libraries(tetris graph graph_match bimap)

find_package(Gperftools)
if (GPERFTOOLS_LIBRARIES AND GPERFTOOLS_INCLUDE_DIR)
        include_directories(${GPERFTOOLS_INCLUDE_DIR})
        add_executable(subiso_profile main.cpp)
        target_link_libraries(subiso_profile graph graph_match bimap ${GPERFTOOLS_LIBRARIES})
endif()

add_executable(expr script/expr.cpp)
target_link_libraries(expr graph graph_match bimap)


include(CTest)
include(FetchContent)
include(GoogleTest)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.11.0
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
        FetchContent_Populate(googletest)
        add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

if(BUILD_TESTING)
        add_subdirectory(test)
endif()
