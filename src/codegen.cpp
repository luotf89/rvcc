#include "logger.h"
#include "utils.h"
#include "codegen.h"
#include "ast.h"
#include "instructions.h"
#include <map>

namespace rvcc {

Ast*& Codegen::ast() {
  return ast_;
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
  if (curr_node->kind() == ExprKind::NODE_NUM ||
        curr_node->kind() == ExprKind::NODE_ID ||
        curr_node->kind() == ExprKind::NODE_NEG ||
        curr_node->kind() == ExprKind::NODE_ADDR ||
        curr_node->kind() == ExprKind::NODE_DEREF ||
        curr_node->kind() == ExprKind::NODE_RETURN ||
        curr_node->kind() == ExprKind::NODE_ASSIGN) { 
    curr_node->codegen();
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

void genAddr(Expr* curr_node) {
  int offset = 0;
  switch (curr_node->kind()) {
    case ExprKind::NODE_ID:
      offset =  -(dynamic_cast<IdentityExpr*>(curr_node)->var()->offset() + 1) * 8;
      addi_("a0", "fp", offset);
      break;
    case ExprKind::NODE_DEREF:
      walkRightImpl(curr_node->getLeft(), codegen_prev_func, codegen_mid_func, codegen_post_func);
      break;
    default:
      FATAL("node kind:  %s not support get addr", curr_node->kindName());
  }
}

}