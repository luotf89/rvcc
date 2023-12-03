cmake_minimum_required(VERSION 3.0.0)
project(rvcc VERSION 0.1.0 LANGUAGES CXX)

include(CTest)
enable_testing()

set(CMAKE_BUILD_TYPE Debug)

add_executable(rvcc main.cpp token.h token.cpp lexer.h lexer.cpp parser.h parser.cpp ast.h ast.cpp )
list(APPEND CFLAGS "-Wall")
target_compile_options(rvcc PRIVATE ${CFLAGS})
set_target_properties(
    rvcc PROPERTIES 
    CXX_STANDARD 14
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
