#include "ast.h"
#include "codegen.h"
#include "logger.h"
#include "parser.h"


using namespace rvcc;

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "args num should be two");
    return -1;
  }
  Logger::getInst().level() = Logger::LogLevel::DEBUG;


  Parser parser(argv[1]);
  Ast* ast = parser.parser_program();
  ast->visualization("graph.dot");
  Codegen* codegen = new Codegen(ast);
  codegen->codegen();

  return 0;
}