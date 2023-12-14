list(APPEND CFLAGS "-Wall")

add_library(ast SHARED
    logger.h logger.cpp
    utils.h utils.cpp
    token.h token.cpp
    lexer.h lexer.cpp
    parser.h parser.cpp
    ast.h ast.cpp
    instructions.h instructions.cpp
    codegen.h codegen.cpp)

add_executable(rvcc main.cpp)

target_link_libraries(rvcc ast)

target_compile_options(rvcc PRIVATE ${CFLAGS})
set_target_properties(
    rvcc PROPERTIES 
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

target_compile_options(ast PRIVATE ${CFLAGS})
set_target_properties(
    ast PROPERTIES 
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)


