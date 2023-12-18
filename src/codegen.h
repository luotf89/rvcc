#ifndef __CODEGEN_H
#define __CODEGEN_H

#include "ast.h"

namespace rvcc {

class Codegen{
  public:
    explicit Codegen(Ast* ast): ast_(ast) {}
    Ast*& ast();
    int compute();
    void codegen();
  private:
    Ast* ast_;
};

}

#endif

