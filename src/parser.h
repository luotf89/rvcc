#ifndef __PARSER_H
#define __PARSER_H

#include "lexer.h"
#include "ast.h"
/*
从表达式中构建ast
表达式比如 1+(2+3)*4+5*(2+3)
有表达式优先级可知 括号 > 乘除 > 加减
因此表达式 主要有两类操作：
一元操作：括号 数值
二元操作：加减乘除
expr = mul(+mul|-mul)*
mul = primary(*primary|/primary)*
primary = val | "("expr")"
*/
namespace rvcc {
  class Parser{
    public:
      Parser(const char* buffer):lexer(buffer){}
      void init();
      AstTree* parser_ast();
    private:
      AstNode* parser_expr();
      AstNode* parser_mul();
      AstNode* parser_primary();
      Lexer lexer;
  };
}
#endif