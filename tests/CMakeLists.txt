include(CTest)
enable_testing()

add_executable(test_ast test_ast.cpp)

target_link_libraries(test_ast ast)

add_test(
  NAME test_ast
  COMMAND $<TARGET_FILE:test_ast>
)
