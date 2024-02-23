#ifndef __CODEGEN_H
#define __CODEGEN_H

#include "ast.h"
#include "object.h"

namespace rvcc {


class Codegen: public Object{
  public:
    explicit Codegen(Ast* ast): ast_(ast) {}
    Ast*& ast();
    void codegen();
  private:
    Ast* ast_;
    static const char* arg_regs[6];
};

bool codegen_prev_func(Expr* curr_node);
bool codegen_mid_func(Expr* curr_node);
bool codegen_post_func(Expr* curr_node);
void genAddr(Expr* curr_node);
void load(Type* type);

} // end namespace rvcc

#endif

