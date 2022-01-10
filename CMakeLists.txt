cmake_minimum_required(VERSION 3.1.1)
project(subiso VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

add_library(graph src/Graph.cpp)
add_library(mapping src/Mapping.cpp)
add_library(graph_match src/GraphMatch.cpp)

add_executable(subiso main.cpp)
target_link_libraries(subiso graph graph_match mapping)

find_package(Gperftools)
if (GPERFTOOLS_LIBRARIES AND GPERFTOOLS_INCLUDE_DIR)
        include_directories(${GPERFTOOLS_INCLUDE_DIR})
        add_executable(subiso_profile main.cpp)
        target_link_libraries(subiso_profile graph graph_match mapping ${GPERFTOOLS_LIBRARIES})
endif()

add_executable(expr script/expr.cpp)
target_link_libraries(expr graph graph_match mapping)


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