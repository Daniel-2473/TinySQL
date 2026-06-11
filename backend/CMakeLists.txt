cmake_minimum_required(VERSION 3.15)
project(mi_servidor)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(Crow)

add_executable(servidor main.cpp)
target_link_libraries(servidor Crow::Crow)