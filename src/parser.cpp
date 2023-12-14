#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "utils.h"
#include "token.h"
#include <cassert>
#include <cstdlib>


using namespace rvcc;

Expr* Parser::binaryOp(Expr* left, Expr*right, ExprType type) {
  return new BinaryExpr(type, 0, left, right);
}

Expr* Parser::unaryOp(Expr*left, ExprType type) {
  return new UnaryExpr(type, 0, left);
}

void Parser::init() {
  lexer_.init();
}

Expr* Parser::parser_program() {
  Expr* program = parser_stmt();
  Expr* curr_stmt = program;
  while(lexer_.getCurrToken().getType() != TokenType::TOKEN_EOF) {
    assert(typeid(*curr_stmt) == typeid(StmtExpr));
    dynamic_cast<StmtExpr*>(curr_stmt)->next() = parser_stmt();
    curr_stmt = curr_stmt->getNext();
  }
  return program;
}

Expr* Parser::parser_stmt() {
  StmtExpr* stmt =  new StmtExpr;
  stmt->type() = ExprType::NODE_STMT;
  stmt->left() = parser_expr();
  if (lexer_.getCurrToken().getType() == TokenType::TOKEN_PUNCT &&
      *(lexer_.getCurrToken().getLoc()) == ';' &&
      lexer_.getCurrToken().getLen() == 1) {
    lexer_.consumerToken();
    return stmt;
  }
  std::cout << "parser stmt failed  expect current token ';' but got "
            << lexer_.getCurrToken().getLoc()
            << std::endl;
  exit(-1);
  return nullptr;;
}

Expr* Parser::parser_expr() {
  Expr* expr = parser_assign();
  return expr;
}

Expr* Parser::parser_assign() {
  Expr* expr = parser_equality();
  while(lexer_.getCurrToken().getLen() == 1 &&
        lexer_.getCurrToken().getType() == TokenType::TOKEN_PUNCT &&
        *(lexer_.getCurrToken().getLoc()) == '=' ) {
    lexer_.consumerToken();
    expr = binaryOp(expr, parser_assign(), ExprType::NODE_ASSIGN);
  }
  return expr;
}

Expr* Parser::parser_equality() {
  Expr* expr = parser_relation();
  while(lexer_.getCurrToken().getLen() == 2 &&
        (Lexer::startWith(lexer_.getCurrToken().getLoc(), "==")  || 
         Lexer::startWith(lexer_.getCurrToken().getLoc(), "!="))) {
    const char* curr_punct = lexer_.getCurrToken().getLoc();
    lexer_.consumerToken();
    if (Lexer::startWith(curr_punct, "==")) {
      expr = binaryOp(expr, parser_relation(), ExprType::NODE_EQ);
    } else {
      expr = binaryOp(expr, parser_relation(), ExprType::NODE_NE);
    }
  }
  return expr;
}

Expr* Parser::parser_relation() {
  Expr* expr = parser_add();
  while((lexer_.getCurrToken().getLen() == 2 && 
         (Lexer::startWith(lexer_.getCurrToken().getLoc(), "<=") ||
          Lexer::startWith(lexer_.getCurrToken().getLoc(), ">="))) ||
        (lexer_.getCurrToken().getLen() == 1 && 
         (Lexer::startWith(lexer_.getCurrToken().getLoc(), "<") ||
          Lexer::startWith(lexer_.getCurrToken().getLoc(), ">")))) {
    const char* curr_punct = lexer_.getCurrToken().getLoc();
    int curr_len = lexer_.getCurrToken().getLen();
    lexer_.consumerToken();
    if (curr_len == 1) {
      if (Lexer::startWith(curr_punct, ">")) {
        expr = binaryOp(parser_add(), expr, ExprType::NODE_LT);
      } else {
        expr = binaryOp(expr, parser_add(), ExprType::NODE_LT);
      }
    } else {
      if (Lexer::startWith(curr_punct, ">=")) {
        expr = binaryOp(parser_add(), expr, ExprType::NODE_LE);
      } else {
        expr = binaryOp(expr, parser_add(), ExprType::NODE_LE);
      }
    }
  }
  return expr;
}

Expr* Parser::parser_add() {
  Expr* expr = parser_mul();
  while(*(lexer_.getCurrToken().getLoc()) == '+' || 
        *(lexer_.getCurrToken().getLoc()) == '-') {
    char punct = *(lexer_.getCurrToken().getLoc());
    lexer_.consumerToken();
    if (punct == '+') {
      expr = binaryOp(expr, parser_mul(), ExprType::NODE_ADD);
    } else {
      expr = binaryOp(expr, parser_mul(), ExprType::NODE_SUB);
    }
  }
  return expr;
}

Expr* Parser::parser_mul() {
  Expr* expr = parser_unary();
  while(*(lexer_.getCurrToken().getLoc()) == '*' || 
        *(lexer_.getCurrToken().getLoc()) == '/') {
    char punct = *(lexer_.getCurrToken().getLoc());
    lexer_.consumerToken();
    if (punct == '*') {
      expr = binaryOp(expr, parser_unary(), ExprType::NODE_MUL);
    } else {
      expr = binaryOp(expr, parser_unary(), ExprType::NODE_DIV);
    }
  }
  return expr;
}

Expr* Parser::parser_unary() {
  Expr* expr;
  if (*(lexer_.getCurrToken().getLoc()) == '+' || 
        *(lexer_.getCurrToken().getLoc()) == '-') {
    char punct = *(lexer_.getCurrToken().getLoc());
    lexer_.consumerToken();
    if (punct == '+') {
      expr = parser_unary();
    } else {
      expr = unaryOp(parser_unary(), ExprType::NODE_NEG);
    }
  } else {
    expr = parser_primary();
  }
  return expr;
}

Expr* Parser::parser_primary() {
  Expr* expr;
  if (lexer_.getCurrToken().getType() == TokenType::TOKEN_NUM) {
    expr = new NumExpr(lexer_.getCurrToken().getValue());
    lexer_.consumerToken();
  } else if (lexer_.getCurrToken().getType() == TokenType::TOKEN_ID) {
    std::size_t hash_value = getstrHash(lexer_.getCurrToken().getLoc(),
                                       lexer_.getCurrToken().getLen());
    Var* var = nullptr;
    if (var_maps_.count(hash_value) == 0) {
      var = new Var(lexer_.getCurrToken().getLoc(), 
                   lexer_.getCurrToken().getLen());
      assert(var_maps_.insert({hash_value, var}).second);
    } else {
      var = var_maps_[hash_value];
    }
    expr = new IdentityExpr(var);
    lexer_.consumerToken();
  } else {
    lexer_.consumerToken();
    expr = parser_expr();
    lexer_.consumerToken();
  }
  return expr;
}

Ast* Parser::parser_ast() {
  init();
  Ast* ast = new Ast;
  ast->root() = new Function();
  ast->root()->body() = parser_program();
  ast->root()->var_maps() = std::move(var_maps_);
  return ast;
}