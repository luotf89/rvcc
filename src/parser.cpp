#include "parser.h"
#include "ast.h"
#include "token.h"

using namespace rvcc;

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
      expr = AstNode::binaryOp(expr, parser_mul(), AstNodeType::NODE_ADD);
    } else {
      expr = AstNode::binaryOp(expr, parser_mul(), AstNodeType::NODE_SUB);
    }
  }
  return expr;
}

AstNode* Parser::parser_mul() {
  AstNode* expr = parser_primary();
  while(*(lexer.getCurrToken().getLoc()) == '*' || 
        *(lexer.getCurrToken().getLoc()) == '/') {
    char punct = *(lexer.getCurrToken().getLoc());
    lexer.consumerToken();
    if (punct == '*') {
      expr = AstNode::binaryOp(expr, parser_mul(), AstNodeType::NODE_MUL);
    } else {
      expr = AstNode::binaryOp(expr, parser_mul(), AstNodeType::NODE_DIV);
    }
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