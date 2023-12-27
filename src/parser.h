#ifndef __PARSER_H
#define __PARSER_H

#include "lexer.h"
#include "ast.h"
#include <cstddef>
/*
program = "{" compound_stmt
compound_stmt = stmt* }
stmt = "return" expr ";" |
       expr? ";" |
       "if" "(" expr ")" stmt ( "else" stmt )? |
       "for" "(" expr? ";" expr? ";" expr? ")" stmt |
       "while" "(" expr ")" stmt |
       "{" compound_stmt
expr = assign
assign = equality (= assign)*
equality =  relation ("==" relation | "!=" relation)*
relation = add ("<" add | ">" add | "<=" add | ">=" add)*
add = mul ("+" mul | "-" mul)*
mul = unary ("*" unary | "/" unary)*
unary = ("+" | "-" | "*" | "&")unary | primary
primary = val | "("expr")"
*/
namespace rvcc {
  class Parser{
    public:
      Parser(const char* buffer):lexer_(buffer), var_idx_(0){}
      void init();
      Ast* parser_ast();
      static Expr* binaryOp(Expr* left, Expr*right, ExprType type);
      static Expr* unaryOp(Expr* left, ExprType type);
    private:
      Expr* parser_program();
      Expr* parser_compound_stmt();
      Expr* parser_stmt();
      Expr* parser_expr();
      Expr* parser_assign();
      Expr* parser_equality();
      Expr* parser_relation();
      Expr* parser_add();
      Expr* parser_mul();
      Expr* parser_unary();
      Expr* parser_primary();
      Lexer lexer_;
      std::map<std::size_t, Var*> var_maps_;
      int var_idx_;
  };
}
#endif