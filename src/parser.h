#ifndef __PARSER_H
#define __PARSER_H

#include "lexer.h"
#include "ast.h"
#include "token.h"
#include "type.h"
#include <cstddef>

namespace rvcc {
  class Parser{
    public:
      Parser(const char* buffer):lexer_(buffer){}
      Ast* parser_program();
      static Expr* binaryOp(Expr* left, Expr*right, ExprKind kind);
      static Expr* unaryOp(Expr* left, ExprKind kind);
    private:
      void init();
      Function* parser_function();
      void parser_parameters(FuncType* func_type);
      void parser_parameter(FuncType* func_type);
      Expr* parser_compound_stmt();
      Expr* parser_declaration();
      Type* parser_declarator(Type* base_type, Token& id);
      Type* parser_suffix(Type* base_type, Token& id);
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
      Expr* parser_call(Token& id);
      Lexer lexer_;
      std::map<std::size_t, Var*> parameter_maps_;
      std::map<std::size_t, Var*> var_maps_;
      int var_index_;
      int var_offset_;
  };
}
#endif