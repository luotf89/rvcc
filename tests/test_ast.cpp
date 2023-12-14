#include "../src/ast.h"
#include "../src/logger.h"
#include "../src/parser.h"
#include "../src/codegen.h"
#include <cassert>

using namespace rvcc;

int main(int argc, char** argv) {

  Logger::getInst().level() = Logger::LogLevel::DEBUG;

  const char* input = "foo2=70; bar4=4; foo2+bar4;";

  Parser parser(input);
  Ast* ast = parser.parser_ast();
  ast->visualization("graph.dot");
  Codegen* codegen = new Codegen(ast);
  assert( codegen->compute()== 74);
  return 0;
}