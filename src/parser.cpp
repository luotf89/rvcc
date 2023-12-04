#include "parser.h"
#include "ast.h"
#include "token.h"

using namespace rvcc;

AstNode* Parser::binaryOp(AstNode* left, AstNode*right, AstNodeType type) {
  AstNode* res = new AstNode;
  res->getLeft() = left;
  res->getRight() = right;
  res->getType() = type;
  return res;
}

AstNode* Parser::unaryOp(AstNode*right, AstNodeType type) {
  AstNode* res = new AstNode;
  res->getRight() = right;
  res->getType() = type;
  return res;
}

void Parser::init() {
  lexer.init();
}

AstNode* Parser::parser_expr() {
  AstNode* expr = parser_mul();
  while(*(lexer.getCurrToken().getLoc()) == '+' || 
        *(lexer.getCurrToken().getLoc()) == '-') {
    char punct = *(lexer.getCurrToken().getLoc());
    lexer.consumerToken();
    if (punct == '+') {
      expr = binaryOp(expr, parser_mul(), AstNodeType::NODE_ADD);
    } else {
      expr = binaryOp(expr, parser_mul(), AstNodeType::NODE_SUB);
    }
  }
  return expr;
}

AstNode* Parser::parser_mul() {
  AstNode* expr = parser_unary();
  while(*(lexer.getCurrToken().getLoc()) == '*' || 
        *(lexer.getCurrToken().getLoc()) == '/') {
    char punct = *(lexer.getCurrToken().getLoc());
    lexer.consumerToken();
    if (punct == '*') {
      expr = binaryOp(expr, parser_unary(), AstNodeType::NODE_MUL);
    } else {
      expr = binaryOp(expr, parser_unary(), AstNodeType::NODE_DIV);
    }
  }
  return expr;
}

AstNode* Parser::parser_unary() {
  AstNode* expr;
  if (*(lexer.getCurrToken().getLoc()) == '+' || 
        *(lexer.getCurrToken().getLoc()) == '-') {
    char punct = *(lexer.getCurrToken().getLoc());
    lexer.consumerToken();
    if (punct == '+') {
      expr = unaryOp(parser_unary(), AstNodeType::NODE_POSITIVE);
    } else {
      expr = unaryOp(parser_unary(), AstNodeType::NODE_NEGATIVE);
    }
  } else {
    expr = parser_primary();
  }
  return expr;
}

AstNode* Parser::parser_primary() {
  AstNode* expr;
  if (lexer.getCurrToken().getType() == TokenType::TOKEN_NUM) {
    expr = new AstNode(AstNodeType::NODE_NUM, lexer.getCurrToken().getValue(), nullptr, nullptr);
    lexer.consumerToken();
  } else {
    lexer.consumerToken();
    expr = parser_expr();
    lexer.consumerToken();
  }
  return expr;
}

AstTree* Parser::parser_ast() {
  init();
  AstTree* ast = new AstTree;
  ast->getRoot() = parser_expr();
  return ast;
}