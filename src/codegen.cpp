#include "logger.h"
#include "type.h"
#include "utils.h"
#include "codegen.h"
#include "ast.h"
#include "instructions.h"
#include <cstddef>
#include <map>
#include <string>

namespace rvcc {

Ast*& Codegen::ast() {
  return ast_;
}

const char* Codegen::arg_regs[6] = {
  "a0", "a1", "a2", "a3", "a4", "a5"
};

void Codegen::codegen_data() {
  for (auto elem: ast_->global_vars()) {
    Var* var = elem.second;
    std::string var_name(var->getName(), var->name_len());
    CHECK(var->addr_kind() == AddrKind::ADDR_DATA);
    printf("  # 数据段标签\n");
    printf("  .data\n");
    printf("  .globl %s\n", var_name.c_str());
    printf("  # 全局变量%s\n", var_name.c_str());
    printf("%s:\n", var_name.c_str());
    printf("  # 零填充%lu位\n", var->type()->size());
    printf("  .zero %lu\n", var->type()->size());
  }
}

/*
当前实现比较low 参数会先把 a1 - a6的值压栈作为local变量来使用
调用者：
  非寄存器参数入栈
  call 被调用者
  非寄存器参数出栈

被调用者：
  ra入栈： ra是调用者调用函数的pc指针的吓一条地址
  fp入栈： fp是 调用者的 栈帧
  sp->fp： 设置被调用者自己的栈帧
  开辟局部变量空间
  执行被调用者函数体内的逻辑
*/
void Codegen::codegen_function() {
  for (auto& elem: ast_->functions()) {
    std::string func_name(elem.second->name(), elem.second->name_len());
    CHECK(elem.second->parameters().size() <= 6);
    std::size_t stack_size = 0;
    // local_vars 包含函数参数
    for (auto& var: elem.second->local_vars()) {
      stack_size += var.second->type()->size();
    }
    stack_size = (stack_size + 16 - 1) / 16 * 16;
    start_(func_name.c_str());
    global_func_name = func_name.c_str();
    // 栈布局
    //-------------------------------// sp
    //              ra
    //-------------------------------// ra = sp-8
    //              fp
    //-------------------------------// fp = sp-16
    //             变量
    //-------------------------------// sp = sp-16-StackSize
    //           表达式计算
    //-------------------------------//

    // Prologue, 前言
    // 将ra寄存器压栈,保存ra的值
    push_("ra");
    push_("fp");
    mv_("fp", "sp");
    printf("  # sp 配分StackSize大小的栈空间\n");
    addi_("sp", "sp", -stack_size);
    printf("\n# ====== 将参数当作局部变量保存在栈空间中=====\n");
    for (auto& param: elem.second->parameters()) {
      int offset = param.second->offset() + param.second->type()->size();
      sd_(arg_regs[param.second->index()], "fp", -offset);
    }
    printf("\n# =====程序主体=====\n");
    elem.second->codegen();
    if (depth != 2) {
      FATAL("depth should be 2 for space ra, fp"
            "but got %d", depth.load());
    }
    return_label_(func_name.c_str());
    mv_("sp", "fp");
    pop_("fp");
    pop_("ra");
    ret_();
  }
}

void Codegen::codegen() {
  codegen_data();
  codegen_function();
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
      {
        Var* var = dynamic_cast<IdentityExpr*>(curr_node)->var();
        std::string var_name(var->getName(), var->name_len());
        if (var->addr_kind() == AddrKind::ADDR_STACK) {
          offset =  -(var->offset() + curr_node->getType()->size());
          printf("  # 获取局部变量%s的栈内地址为%d(fp)\n", var_name.c_str(),
              offset);
          addi_("a0", "fp", offset);
        } else if (var->addr_kind() == AddrKind::ADDR_DATA) {
          printf("  # 获取全局变量%s的地址\n", var_name.c_str());
          printf("  lui a0, %%hi(%s)\n", var_name.c_str());
          printf("  addi a0, a0, %%lo(%s)\n", var_name.c_str());
          // printf("  la a0, %s\n", var_name.c_str());
        } else {
          FATAL("current address type: %s is not support", var->getAddrKindName());
        }
        break;
      }
    case ExprKind::NODE_DEREF:
      {
        walkRightImpl(curr_node->getLeft(), codegen_prev_func, codegen_mid_func, codegen_post_func);
        break;
      }
    default:
      FATAL("node kind:  %s not support get addr", curr_node->kindName());
  }
}

void load(Type* type) {
  if (type->kind() == TypeKind::TYPE_ARRAY) {
    return;
  }
  ld_("a0", "a0", 0);
}

}