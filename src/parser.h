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
      static Expr* newAdd(Expr* left, Expr* right);
      static Expr* newSub(Expr* left, Expr* right);
      static void updatePtrOffset(Expr*& left, Expr*& right);
    private:
      void init();
      Function* parser_function(Type* type, Token& id);
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
      Expr* parser_postfix();
      Expr* parser_primary();
      Expr* parser_call(Token& id);
      Lexer lexer_;
      std::map<std::size_t, Var*> parameter_maps_;
      std::map<std::size_t, Var*> local_vars_;
      std::map<std::size_t, Var*> global_vars_;
      int local_var_index_;
      int local_var_offset_;
      int global_var_index_;
      int global_var_offset_;
  };
}
#endif