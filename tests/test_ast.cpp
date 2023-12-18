#include "../src/ast.h"
#include "../src/logger.h"
#include "../src/parser.h"
#include "../src/codegen.h"
#include <cassert>

using namespace rvcc;

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "args num should be two");
    return -1;
  }

  Logger::getInst().level() = Logger::LogLevel::DEBUG;

  Parser parser(argv[1]);
  Ast* ast = parser.parser_ast();
  ast->visualization("graph.dot");
  Codegen* codegen = new Codegen(ast);
  auto ret = codegen->compute();
  return ret;
}