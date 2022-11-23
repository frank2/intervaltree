cmake_minimum_required(VERSION 3.10)

project(libintervaltree)
option(TEST_INTERVALTREE "Enable testing for IntervalTree." OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/avltree)

file(GLOB_RECURSE HEADER_FILES FOLLOW_SYMLINKS ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/include/*.hpp)
source_group(TREE "${PROJECT_SOURCE_DIR}" PREFIX "Header Files" FILES ${HEADER_FILES})
add_library(libintervaltree INTERFACE)
target_link_libraries(libintervaltree INTERFACE libavltree)
target_include_directories(libintervaltree INTERFACE
  "${PROJECT_SOURCE_DIR}/include"
)

if (TEST_INTERVALTREE)
  enable_testing()
  add_executable(testintervaltree ${PROJECT_SOURCE_DIR}/test/main.cpp ${PROJECT_SOURCE_DIR}/test/framework.hpp)
  target_link_libraries(testintervaltree PUBLIC libintervaltree)
  target_include_directories(testintervaltree PUBLIC
    "${PROJECT_SOURCE_DIR}/test"
  )
  add_test(NAME testintervaltree COMMAND testintervaltree)
endif()
