cmake_minimum_required(VERSION 3.25.0)
project(SPMM VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(SPMM main.cpp)

include_directories(include)
add_subdirectory(matrix)

target_link_libraries(SPMM MATRIX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
