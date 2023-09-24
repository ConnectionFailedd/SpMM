cmake_minimum_required(VERSION 3.25.0)
project(SPMM VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(SPMM main.cpp)
add_executable(TESTCASE_GEN testcase_gen.cpp)

include_directories(include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
