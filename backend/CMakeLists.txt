cmake_minimum_required(VERSION 3.20)
project(TinySQL)

set(CMAKE_CXX_STANDARD 20)

add_executable(TinySQL main.cpp)

target_include_directories(TinySQL PUBLIC
    ${CMAKE_SOURCE_DIR}/boost_1_91_0
    ${CMAKE_SOURCE_DIR}/Crow-1.3.2/include
    /usr/include
)

target_compile_definitions(TinySQL PUBLIC CROW_USE_BOOST)