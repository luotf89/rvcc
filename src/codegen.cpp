#include "logger.h"
#include "utils.h"
#include "codegen.h"
#include "ast.h"
#include "instructions.h"
#include <cassert>
#include <map>

namespace rvcc {

Ast*& Codegen::ast() {
  return ast_;
}

int Codegen::compute() {
  return ast_->computer();
}

void Codegen::codegen() {
  int stack_size = ast_->root()->var_maps().size();
  start_();
  push_("fp");
  mv_("fp", "sp");
  printf("  # sp 配分StackSize大小的栈空间\n");
  addi_("sp", "sp", -8*stack_size);
  printf("\n# =====程序主体===============\n");
  ast_->codegen();
  if (depth != 1) {
    FATAL("depth should be 0 but got %d", depth.load());
  }
  return_label_();
  mv_("sp", "fp");
  pop_("fp");
  ret_();
}

bool codegen_prev_func(Expr* curr_node) {
  if (curr_node->type() == ExprType::NODE_NUM ||
        curr_node->type() == ExprType::NODE_ID ||
        curr_node->type() == ExprType::NODE_ASSIGN ||
        curr_node->type() == ExprType::NODE_NEG ||
        curr_node->type() == ExprType::NODE_RETURN) {
    if (curr_node->type() == ExprType::NODE_NUM ||
        curr_node->type() == ExprType::NODE_ID) {
      curr_node->codegen();
    } else if (curr_node->type() == ExprType::NODE_NEG ||
               curr_node->type() == ExprType::NODE_RETURN){
      walkRightImpl(curr_node->getLeft(), codegen_prev_func, codegen_mid_func, codegen_post_func);
      curr_node->codegen();
    } else {
      walkRightImpl(curr_node->getRight(), codegen_prev_func, codegen_mid_func, codegen_post_func);
      curr_node->codegen();
    }
    return false;
  }
  return true;
}

bool codegen_mid_func(Expr* curr_node) {
  push_("a0");
  return true;
}

bool codegen_post_func(Expr* curr_node) {
  pop_("a1");
  curr_node->codegen();
  return true;
}

}