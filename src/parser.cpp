#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "utils.h"
#include "token.h"
#include <cassert>
#include <cstdlib>


using namespace rvcc;

Expr* Parser::binaryOp(Expr* left, Expr*right, ExprType type) {
  return new BinaryExpr(type, left, right);
}

Expr* Parser::unaryOp(Expr*left, ExprType type) {
  return new UnaryExpr(type, left);
}

void Parser::init() {
  lexer_.init();
}

Expr* Parser::parser_program() {
  rvcc::startWithStr("{", "program", lexer_);
  lexer_.consumerToken();
  Expr* program = parser_compound_stmt();
  return program;
}

Expr* Parser::parser_compound_stmt() {
  CompoundStmtExpr* compound_stmt = new CompoundStmtExpr();
  NextExpr* head = new StmtExpr();
  NextExpr* curr_stmt = head; 
  while(!(startWithStr("}", lexer_) ||
          lexer_.getCurrToken().getType() == TokenType::TOKEN_ILLEGAL ||
          lexer_.getCurrToken().getType() == TokenType::TOKEN_EOF)) {
    curr_stmt->next() = parser_stmt();
    curr_stmt = dynamic_cast<NextExpr*>(curr_stmt->getNext());
  }
  startWithStr("}", compound_stmt, lexer_);
  lexer_.consumerToken();
  compound_stmt->stmts() = head->getNext();
  head->next() = nullptr;
  delete head;
  return compound_stmt;
}

Expr* Parser::parser_stmt() {
  Expr* stmt;
  
  if (startWithStr("return", lexer_)) {
    lexer_.consumerToken();
    stmt =  new StmtExpr;
    dynamic_cast<StmtExpr*>(stmt)->left() =
      unaryOp(parser_expr(), ExprType::NODE_RETURN);
    startWithStr(";", stmt, lexer_);
    lexer_.consumerToken();
  } else if (startWithStr("{", lexer_)) {
    lexer_.consumerToken();
    stmt = parser_compound_stmt();
  } else if (startWithStr("if", lexer_)) {
    lexer_.consumerToken();
    IfExpr* if_stmt = new IfExpr();
    if (startWithStr("(", if_stmt, lexer_)) {
      lexer_.consumerToken();
      if_stmt->cond() = parser_expr();
      startWithStr(")", if_stmt, lexer_);
      lexer_.consumerToken();
      if_stmt->then() = parser_stmt();
      if (startWithStr("else", lexer_)) {
        lexer_.consumerToken();
        if_stmt->els() = parser_stmt();
      }
    }
    stmt = if_stmt;
  } else if (startWithStr("for", lexer_)) {
    lexer_.consumerToken();
    ForExpr* for_stmt = new ForExpr();
    if (startWithStr("(", for_stmt, lexer_)) {
      lexer_.consumerToken();
      if (!startWithStr(";", lexer_)) {
        for_stmt->init() = parser_expr();
      }
      startWithStr(";", for_stmt, lexer_);
      lexer_.consumerToken();
      if (!startWithStr(";", lexer_)) {
        for_stmt->cond() = parser_expr();
      }
      startWithStr(";", for_stmt, lexer_);
      lexer_.consumerToken();
      if (!startWithStr(")", lexer_)) {
        for_stmt->inc() = parser_expr();
      }
      startWithStr(")", for_stmt, lexer_);
      lexer_.consumerToken();
      for_stmt->stmts() = parser_stmt();
    }
    stmt = for_stmt;
  } else if (startWithStr("while", lexer_)) {
    lexer_.consumerToken();
    WhileExpr* while_stmt = new WhileExpr();
    if (startWithStr("(", while_stmt, lexer_)) {
      lexer_.consumerToken();
      while_stmt->cond() = parser_expr();
      startWithStr(")", while_stmt, lexer_);
      lexer_.consumerToken();
      while_stmt->stmts() = parser_stmt();
    }
    stmt = while_stmt;
  } else {
    // 空语句的处理逻辑 ;;
    if (startWithStr(";", lexer_)) {
      lexer_.consumerToken();
      stmt = new CompoundStmtExpr;
      return stmt;
    }
    stmt =  new StmtExpr;
    stmt->type() = ExprType::NODE_STMT;
    dynamic_cast<StmtExpr*>(stmt)->left() = parser_expr();
    startWithStr(";", stmt, lexer_);
    lexer_.consumerToken();
  }
  return stmt;
}

Expr* Parser::parser_expr() {
  Expr* expr = parser_assign();
  return expr;
}

Expr* Parser::parser_assign() {
  Expr* expr = parser_equality();
  while(startWithStr("=", lexer_) ) {
    lexer_.consumerToken();
    expr = binaryOp(expr, parser_assign(), ExprType::NODE_ASSIGN);
  }
  return expr;
}

Expr* Parser::parser_equality() {
  Expr* expr = parser_relation();
  while(startWithStr("==", lexer_) ||
        startWithStr("!=", lexer_)) {
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
  while(startWithStr("<=", lexer_) ||
        startWithStr(">=", lexer_) ||
        startWithStr("<", lexer_) ||
        startWithStr(">", lexer_)) {
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
  while(startWithStr("+", lexer_) || 
        startWithStr("-", lexer_)) {
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
  while(startWithStr("*", lexer_) || 
        startWithStr("/", lexer_)) {
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
  if (startWithStr("+", lexer_) || 
      startWithStr("-", lexer_)) {
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
  ast->root()->getVarOffsets();
  return ast;
}