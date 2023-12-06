#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "token.h"
#include <cstdlib>

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

AstNode* Parser::parser_program() {
  AstNode* program = parser_stmt();
  while(lexer.getCurrToken().getType() != TokenType::TOKEN_EOF) {
    program->getNext() = parser_stmt();
  }
  return program;
}

AstNode* Parser::parser_stmt() {
  AstNode* stmt =  new AstNode();
  stmt->getType() = AstNodeType::NODE_STMT;
  stmt->getLeft() = parser_expr();
  if (lexer.getCurrToken().getType() == TokenType::TOKEN_PUNCT &&
      *(lexer.getCurrToken().getLoc()) == ';' &&
      lexer.getCurrToken().getLen() == 1) {
    lexer.consumerToken();
    return stmt;
  }
  std::cout << "parser stmt failed  expect current token ';' but got "
            << lexer.getCurrToken().getLoc()
            << std::endl;
  exit(-1);
  return nullptr;;
}

AstNode* Parser::parser_expr() {
  AstNode* expr = parser_relation();
  while(lexer.getCurrToken().getLen() == 2 &&
        (Lexer::startWith(lexer.getCurrToken().getLoc(), "==")  || 
         Lexer::startWith(lexer.getCurrToken().getLoc(), "!="))) {
    const char* curr_punct = lexer.getCurrToken().getLoc();
    lexer.consumerToken();
    if (Lexer::startWith(curr_punct, "==")) {
      expr = binaryOp(expr, parser_relation(), AstNodeType::NODE_EQ);
    } else {
      expr = binaryOp(expr, parser_relation(), AstNodeType::NODE_NE);
    }
  }
  return expr;
}

AstNode* Parser::parser_relation() {
  AstNode* expr = parser_add();
  while((lexer.getCurrToken().getLen() == 2 && 
         (Lexer::startWith(lexer.getCurrToken().getLoc(), "<=") ||
          Lexer::startWith(lexer.getCurrToken().getLoc(), ">="))) ||
        (lexer.getCurrToken().getLen() == 1 && 
         (Lexer::startWith(lexer.getCurrToken().getLoc(), "<") ||
          Lexer::startWith(lexer.getCurrToken().getLoc(), ">")))) {
    const char* curr_punct = lexer.getCurrToken().getLoc();
    int curr_len = lexer.getCurrToken().getLen();
    lexer.consumerToken();
    if (curr_len == 1) {
      if (Lexer::startWith(curr_punct, ">")) {
        expr = binaryOp(parser_add(), expr, AstNodeType::NODE_LT);
      } else {
        expr = binaryOp(expr, parser_add(), AstNodeType::NODE_LT);
      }
    } else {
      if (Lexer::startWith(curr_punct, ">=")) {
        expr = binaryOp(parser_add(), expr, AstNodeType::NODE_LE);
      } else {
        expr = binaryOp(expr, parser_add(), AstNodeType::NODE_LE);
      }
    }
  }
  return expr;
}

AstNode* Parser::parser_add() {
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
      expr = parser_unary();
    } else {
      expr = unaryOp(parser_unary(), AstNodeType::NODE_NEG);
    }
  } else {
    expr = parser_primary();
  }
  return expr;
}

AstNode* Parser::parser_primary() {
  AstNode* expr;
  if (lexer.getCurrToken().getType() == TokenType::TOKEN_NUM) {
    expr = new AstNode(AstNodeType::NODE_NUM, lexer.getCurrToken().getValue(),
                  nullptr, nullptr, nullptr);
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
  ast->getRoot() = parser_program();
  return ast;
}