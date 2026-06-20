cmake_minimum_required(VERSION 3.20)
project(TinySQL)
set(CMAKE_CXX_STANDARD 20)

# Agrega todos los archivos .cpp que se van a compilar en el ejecutable
add_executable(TinySQL
    main.cpp
    SystemCatalog.cpp
    src/api/Router.cpp
    src/api/handlers/QueryHandler.cpp
    src/query/Lexer.cpp                  
    src/query/Parser.cpp                 
    src/query/SemanticValidator.cpp     
)

target_include_directories(TinySQL PUBLIC
    ${CMAKE_SOURCE_DIR}/boost_1_91_0
    ${CMAKE_SOURCE_DIR}/Crow-1.3.2/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}                
    /usr/include
)

target_compile_definitions(TinySQL PUBLIC 
    CROW_USE_BOOST
    CROW_RETURNS_OK_ON_HTTP_OPTIONS_REQUEST
)

if(WIN32)
    target_link_libraries(TinySQL PUBLIC ws2_32 mswsock)
endif()