#include "codegen.h"
#include "ast.h"
#include "instructions.h"
#include <map>

using namespace rvcc;

int Codegen::compute() {
  return ast_->computer();
}

void Codegen::codegen() {
  int stack_size = ast_->root()->var_maps().size();
  ast_->root()->getVarOffsets();
  start_();
  push_("fp");
  mv_("fp", "sp");
  addi_("sp", "sp", -8*stack_size);
  ast_->codegen();
  pop_("a0");
  mv_("sp", "fp");
  pop_("fp");
  ret_();
}