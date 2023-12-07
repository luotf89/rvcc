#ifndef __PARSER_H
#define __PARSER_H

#include "lexer.h"
#include "ast.h"
/*
program = (stmt)+
stmt = expr ";"
expr = assign
assign = equality (= assign)*
equality =  relation ("==" relation | "!=" relation)*
relation = add ("<" add | ">" add | "<=" add | ">=" add)*
add = mul ("+" mul | "-" mul)*
mul = unary ("*" unary | "/" unary)*
unary = ("+" | "-" )unary | primary
primary = val | "("expr")"
*/
namespace rvcc {
  class Parser{
    public:
      Parser(const char* buffer):lexer(buffer){}
      void init();
      AstTree* parser_ast();
      static AstNode* binaryOp(AstNode* left, AstNode*right, AstNodeType type);
      static AstNode* unaryOp(AstNode*right, AstNodeType type);
    private:
      AstNode* parser_program();
      AstNode* parser_stmt();
      AstNode* parser_expr();
      AstNode* parser_assign();
      AstNode* parser_equality();
      AstNode* parser_relation();
      AstNode* parser_add();
      AstNode* parser_mul();
      AstNode* parser_unary();
      AstNode* parser_primary();
      Lexer lexer;
  };
}
#endif