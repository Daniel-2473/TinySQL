cmake_minimum_required(VERSION 3.20)
project(TinySQL)
set(CMAKE_CXX_STANDARD 20)

# Lista todos los .cpp del proyecto
add_executable(TinySQL
    main.cpp
    src/api/Router.cpp
    src/api/handlers/QueryHandler.cpp
)

# Carpetas donde el compilador busca los #include
target_include_directories(TinySQL PUBLIC
    ${CMAKE_SOURCE_DIR}/boost_1_91_0
    ${CMAKE_SOURCE_DIR}/Crow-1.3.2/include
    ${CMAKE_SOURCE_DIR}/src          # para que encuentre api/, models/, etc.
    /usr/include
)

target_compile_definitions(TinySQL PUBLIC CROW_USE_BOOST)