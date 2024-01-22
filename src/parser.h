#ifndef __PARSER_H
#define __PARSER_H

#include "lexer.h"
#include "ast.h"
#include "type.h"
#include <cstddef>
/*
program = functionDefinition*
functionDefinition = declspec declarator "{" compoundStmt*
declspec = "int"
declarator = "*"* ident typeSuffix
typeSuffix = ("(" ")")?

compound_stmt = (declaration | stmt)* "}"

declaration = declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
declspec = "int"
declarator = "*"* ident

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
primary = num | "("expr")" | var args?
args = "(" ")"
*/
namespace rvcc {
  class Parser{
    public:
      Parser(const char* buffer):lexer_(buffer), var_idx_(0){}
      Ast* parser_program();
      static Expr* binaryOp(Expr* left, Expr*right, ExprKind kind);
      static Expr* unaryOp(Expr* left, ExprKind kind);
    private:
      void init();
      Function* parser_function();
      Expr* parser_compound_stmt();
      Expr* parser_declaration();
      Type* parser_declarator(Type* base_type);
      Type* parser_declspec();
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